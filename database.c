#include "server.h"

MYSQL_RES *sql_result;
MYSQL_ROW sql_row;
MYSQL *conn;

// 쿼리 실행
int runQuery(char *query)
{
        if(mysql_query(conn, query)) {
                DB_errMsg("MySQL Query Excute failed");
                return -1;
        }

        sql_result = mysql_store_result(conn);
        return 0;
}

//로우 조회 , sql_result에 query로 받은 row들을 실행때마다 차례대로 받아옴
int fetchRow(void)
{
        if(sql_result) {
                sql_row = mysql_fetch_row(sql_result);
                if(!sql_row) {
                        DB_errMsg("MySQL Fetch failed");
                        return -1;
                }
                return 1;
        }
        DB_errMsg("MySQL Query Result Null");
        return -2;
}

// 디비 연결
int connectDB(void)
{
        char *server = "localhost";
        char *user = "root";
        char *password = "1234";
        char *database = "darktemplar";

        conn = mysql_init(NULL);
        if (conn == NULL) {
                DB_errMsg("MySQL init fail");
                return -1;
        }
        if(!mysql_real_connect(conn, server,
           user, password, database, 0, NULL, 0)) {
                DB_errMsg("MySQL connect fail");
                return -2;
        }
        return 0;

}

// 디비 연결 해제
void closeDB(void)
{
        if (sql_result)
                mysql_free_result(sql_result);
        mysql_close(conn);
}

//유저나 방의 개수를 호출마다 하나씩 추가한다.
int addCount(char *cntType)
{
	int cnt;
	char query[255] = {0}; // 임시로 담을 버퍼

	runQuery("Select * from count");
	fetchRow();
    cnt = atoi(sql_row[0]) + 1; // 유저 수 하나 추가
    snprintf(query, 255, "update count SET %s = %d", cntType, cnt);
    runQuery(query); // 갱신

	return cnt;
}

// Roomcount 조회
int inquiryRcount()
{
    runQuery("Select * from count");
    fetchRow();
    LogNum("RoomCount",atoi(sql_row[1]));
    return atoi(sql_row[1]);
}
// sid로 유저id를 찾아온다.
char* convertSid(int sid)
{
	runQuery("Select * from count");
	fetchRow();
    int ucnt = atoi(sql_row[0]);
	printf("ucnt = %d \n", ucnt);
	runQuery("Select * from Userinfo"); // 비교대상을 가져옴

	for(int i=0; i < ucnt; i++)
	{
	    fetchRow(); // 검사 진행
		if (sid == atoi(sql_row[0]))
			return sql_row[1];
	}
	return NULL; // 유저id를 찾지 못함
}

//유저 회원가입을 시킴, sid는 자동으로 0부터 1씩 추가된다, 유저의 수도 늘어남
int joinMembership(char *id, char *pwd)
{

    int ucnt;
	char query[255] = {0}; // 임시로 담을 버퍼

    memset(query, 0x00, sizeof(query));
	snprintf(query, 255,"insert into Userinfo (id, pwd) values('%s', '%s')", id, pwd);
	if (runQuery(query) == -1) // 아이디 중복
        return 0;

	addCount("ucnt");

	if (sql_result)
                mysql_free_result(sql_result);

    Log("join Success");

    return 1;
}

int compareID(char *id, char *pwd,struct profile *Upf,int * sid) // ID, PWD를 비교한다. TRUE면 일치, FALSE면 일치하는 정보가 없다(틀리는것도 해당)
{ // ucnt는 단순히 반복문 수행을 위해서 유저 수를 체크하는 변수
    runQuery("Select * from count");
	fetchRow();
    int ucnt = atoi(sql_row[0]);
    LogNum("ucnt",ucnt);
	runQuery("Select * from Userinfo"); // 비교대상을 가져옴


	for(int i=0; i < ucnt; i++)
	{
	    fetchRow(); // 검사 진행
		if ((!strcmp(sql_row[1],id))&&(!strcmp(sql_row[2],pwd))) { // 유저의 id,pwd가 일치하는 상황
			//유저의 정보를 보내기위해 불러온 값을 저장한다.
			*sid = atoi(sql_row[0]);
			strncpy(Upf->id, sql_row[1], sizeof(Upf->id));
			Upf->win = atoi(sql_row[3]);
			Upf->lose = atoi(sql_row[4]);
			Upf->kill = atoi(sql_row[5]);
			Upf->death = atoi(sql_row[6]);


            return 1;
		}
        else
			continue;
	}
    *sid = -1; // login fail
	return 0;
}

// 방 생성시 list에 추가(방 번호, 호스트 이름)
void createRoom(int sid, struct room *Rinfo)
{
    int rcnt;
	char query[255] = {0}; // 임시로 담을 버퍼
//	char idBuf[16] = {0};

	//strncpy(idBuf, convertSid(sid), sizeof(idBuf));
	snprintf(query, 255, "insert into RoomList (rid, hname)values('%d', '%s')", addCount("rcnt"), convertSid(sid));
	runQuery(query);

    rcnt = inquiryRcount();

    Rinfo->rid = rcnt;
    Rinfo->ucount = 1;

	if (sql_result)
                mysql_free_result(sql_result);

    Log("Create Room Success");
}

// 방 목록을 가져온다.
void bringRoomList(struct lobbyListAuth *Rbuffer[],int divrcnt, int modrcnt) //
{
	int  i = 0, j; //,rcnt;

//    rcnt = inquiryRcount();

	runQuery("Select * from RoomList"); // 방 목록 가져옴

   for(i = 0;i < divrcnt; i++)
    { // 방 목록 전송 과정 1
        fetchRow();
        for(j = 0; j < 4; j++)
            {
            Rbuffer[i]->room[j].rid = atoi(sql_row[0]);
            Rbuffer[i]->room[j].ucount = atoi(sql_row[1]);
            strncpy(Rbuffer[i]->room[j].hname, sql_row[2], sizeof(Rbuffer[i]->room[j].hname));
            }
    }
    for (i = 0; i < modrcnt; i++)  // 방 목록 전송 과정 2 0~3개 남은 방들을 담아 전송
    {
         fetchRow();
         Rbuffer[divrcnt]->room[i].rid = atoi(sql_row[0]);
         Rbuffer[divrcnt]->room[i].ucount = atoi(sql_row[1]);
         strncpy(Rbuffer[divrcnt]->room[i].hname, sql_row[2], sizeof(Rbuffer[divrcnt]->room[i].hname));
    }

}

// 방 접속시 호출, 방을 갱신하고 다른 유저의 정보를 불러옴, 방장 + host1,2,3 구조
int enterRoom(int rid, int sid, struct lobbyEnterAuth *RoomUser)
{
    int ucnt, i, j;
    char roomHostId[16] = {0}; // 방장의 id를 넣을 임시 버퍼
    char query[255] = {0}; // query 진행용 임시 버퍼
    int host[3] = {0}; // host 1,2,3의 sid를 넣을 임시 버퍼

    // 해당 방 번호에 있는 유저의 기록을 불러온다
    snprintf(query, 255, "Select * from RoomList where rid = %d",rid);
    if (runQuery(query) == -1)
    {
        Log("EnterRoom failed");
        return 0; // 일치하는 rid가 없음
    }
    fetchRow();

	//host 들의 sid와 방장의 id, 방에 있는 유저 수 조회
    ucnt = atoi(sql_row[1]); // 방에 있는 유저 수
    strncpy(roomHostId, sql_row[2], sizeof(roomHostId)); // 방장의 id를 가져옴
    for(i = 0; i < 3; i++) // host 1,2,3 sid 가져옴
    {
        host[i] = atoi(sql_row[i+3]);
    }
    //방장의 정보를 user[0]에 대입
    snprintf(query, 255, "Select * from Userinfo where id = '%s'", roomHostId);
    fetchRow();
    strncpy(RoomUser->user[0].id, sql_row[1], sizeof(RoomUser->user[0].id));
    RoomUser->user[0].slot = 0; // 슬롯에 배치되었다.
    RoomUser->user[0].win = atoi(sql_row[3]);
    RoomUser->user[0].lose = atoi(sql_row[4]);
    RoomUser->user[0].kill = atoi(sql_row[5]);
    RoomUser->user[0].death = atoi(sql_row[6]);

    // 방장을 제외한 host 1,2,3의 정보를 넣는 과정
    for(i = 0; i < 3; i++)
    {// 처음 빈칸이 나오면 들어올 host의 정보를 삽입하고 다음 빈칸은 그냥 continue
        if(host[i] == 0) // 이 slot은 비어있음, 접속한 유저를 삽입한다.
        {
            snprintf(query, 255, "update RoomList set host%d = %d where rid = %d", i+2, sid, rid);
            runQuery(query);
            break;
        }
    }
	for(i = 0; i <3; i++)
	{
		if(host[i] == 0) // 다음 빈칸이므로 continue
			continue;

		else // 유저 정보를 대입한다, segmentation fault를 막기위해 대입 후 다시 for을 돌려 유저 정보를 다시 대입한다.
		{
			snprintf(query, 255, "Select * from Userinfo where sid = %d", host[i]);
			fetchRow();
			strncpy(RoomUser->user[1].id, sql_row[1], sizeof(RoomUser->user[1].id));
			RoomUser->user[1].slot = 0; // 슬롯에 배치되었다.
			RoomUser->user[1].win = atoi(sql_row[3]);
			RoomUser->user[1].lose = atoi(sql_row[4]);
			RoomUser->user[1].kill = atoi(sql_row[5]);
			RoomUser->user[1].death = atoi(sql_row[6]);
			break;
		}
	}
	for(i; i <3; i++)
	{
		if(host[i] == 0) // 다음 빈칸이므로 continue
			continue;
		else
		{
			snprintf(query, 255, "Select * from Userinfo where sid = %d", host[i]);
			fetchRow();
			strncpy(RoomUser->user[2].id, sql_row[1], sizeof(RoomUser->user[2].id));
			RoomUser->user[2].slot = 0; // 슬롯에 배치되었다.
			RoomUser->user[2].win = atoi(sql_row[3]);
			RoomUser->user[2].lose = atoi(sql_row[4]);
			RoomUser->user[2].kill = atoi(sql_row[5]);
			RoomUser->user[2].death = atoi(sql_row[6]);
		}
	}

    // 들어가려는 방의 ucount 갱신 과정, host 삽입
    snprintf(query, 255, "update RoomList set ucount = %d where rid = %d", ucnt+1, rid);
    runQuery(query);

	Log("EnterRoom Success!");
	return 1; // 성공적으로 마침
}

void DB_errMsg(char *errMsg)
{
        printf("%s\n",errMsg);
        fprintf(stderr, "Error Meassage : %s \n", mysql_error(conn));
}

