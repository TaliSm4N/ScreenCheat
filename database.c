#include "server.h"

MYSQL_RES *sql_result;
MYSQL_ROW sql_row;
MYSQL *conn;

char query[255] = {0}; // 임시로 담을 버퍼

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

// 유저, 방 수 조회
int inquiryCount(char *cntTable)
{
	int cnt;

    memset(query, 0x00, sizeof(query));
	snprintf(query, 255, "select count(*) from %s", cntTable);
	runQuery(query);
	fetchRow();
    cnt = atoi(sql_row[0]); // 유저, 방 수 조회

	return cnt;
}

int inquiryHostfd(int rid)
{
    int i;
    memset(query, 0x00, sizeof(query));

    snprintf(query, 255, "Select hostfd from RoomList where rid = %d",rid);
	runQuery(query); // 오류 체크용으로 임시로 박아둠
	fetchRow();

	Log("inquiryHostfd success!");
    return atoi(sql_row[0]);
}

// sid로 유저id를 찾아온다.
char* convertUid(int uid)
{
    int ucnt = inquiryCount("Userinfo"); // 유저 수 조회
	printf("ucnt = %d \n", ucnt);
	runQuery("Select * from Userinfo"); // 비교대상을 가져옴

	for(int i=0; i < ucnt; i++)
	{
	    fetchRow(); // 검사 진행
		if (uid == atoi(sql_row[0]))
			return sql_row[1];
	}
	return NULL; // 유저id를 찾지 못함
}

// uid에 맞는 유저의 정보를 가져옴
void bringUserinfo(int uid, struct user *updateUser)
{
	Log("Bring Userinfomation");
    memset(query, 0x00, sizeof(query));
    snprintf(query, 255, "select * from Userinfo where uid = %d", uid);
    runQuery(query);
    fetchRow();
    strncpy(updateUser->id, sql_row[1], sizeof(updateUser->id));
    updateUser->win = atoi(sql_row[3]);
    updateUser->lose  = atoi(sql_row[4]);
    updateUser->kill = atoi(sql_row[5]);
    updateUser->death = atoi(sql_row[6]);
}

// rid가 중간에 빈게 있는지 체크한다.
int checkRoomid(void)
{
	int i = 1, rcnt = inquiryCount("RoomList");
	Log("Check Roomid");
	memset(query, 0x00, sizeof(query));
	runQuery("select rid from RoomList");

	for(i = 1; i <= rcnt; i++)
	{
		fetchRow();
		if (i != atoi(sql_row[0])) // 중간 숫자가 빈 경우
		{
		    return i;
		}

	}

	return i; // 끝까지 마친값
}

//유저 회원가입을 시킴, uid는 1씩 추가된다
int joinMembership(char *id, char *pwd)
{
    int ucnt = inquiryCount("Userinfo") + 1; // uid = ucnt + 1
    LogNum("ucnt + 1",ucnt);
    memset(query, 0x00, sizeof(query));
	snprintf(query, 255,"insert into Userinfo (uid,id, pwd) values('%d' ,'%s', '%s')", ucnt, id, pwd);
	if (runQuery(query) == -1) // 아이디 중복
        return 0;

	if (sql_result)
                mysql_free_result(sql_result);

    Log("join Success");

    return 1;
}

int compareID(char *id, char *pwd, struct profile *Upf, int * uid) // ID, PWD를 비교한다. TRUE면 일치, FALSE면 일치하는 정보가 없다(틀리는것도 해당)
{
    int ucnt = inquiryCount("Userinfo"); // 유저 수 조회

	runQuery("Select * from Userinfo"); // 비교대상을 가져옴


	for(int i=0; i < ucnt; i++)
	{
	    fetchRow(); // 검사 진행
		if ((!strcmp(sql_row[1],id))&&(!strcmp(sql_row[2],pwd))) { // 유저의 id,pwd가 일치하는 상황
			//유저의 정보를 보내기위해 불러온 값을 저장한다.
			*uid = atoi(sql_row[0]);
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
	// login fail
    //*uid = -1;
	return 0;
}

// 방 생성시 list에 추가(방 번호, 호스트 이름,호스트 fd)
void createRoom(int uid, int fd, struct lobbyCreateAuth *Rinfo)
{
	int rid;
    rid = checkRoomid();
	LogNum("rid", rid);
	memset(query, 0x00, sizeof(query));
	snprintf(query, 255, "insert into RoomList (rid, hname,hostfd) values('%d', '%s','%d')", rid, convertUid(uid), fd);
	runQuery(query);

    Rinfo->rid = rid;
	if (sql_result)
                mysql_free_result(sql_result);

    Log("Create Room Success");
}

// 방 퇴장시 인원 update, 방장이 아닌 다른 인원에 대해서만 update가 진행된다
void exitRoom(int fd, int rid, int *broadcastfd)
{
    int i, ucnt;

    // 해당 rid에 ucnt 를 가져오는 과정
    snprintf(query, 255, "Select ucount from RoomList where rid = %d", rid);
	runQuery(query);
	fetchRow();
	ucnt = atoi(sql_row[1]) - 1; // 방에 있는 유저 수에서 1 뺌

    memset(query, 0x00, sizeof(query));
    for(i = 1; i < 4; i++)
    {
        if(broadcastfd[i] == fd) // 삭제 대상과 일치
        {
            LogNum("exitRoom fd", fd);
            snprintf(query, 255, "update RoomList set ucount = %d, guest%d = 0, guest%dfd = 0 where rid = %d", ucnt, i+1, i+1, rid);
            runQuery(query);
        }
    }
    Log("exitRoom update Success");
}

// 방 목록을 가져온다.
void bringRoomList(struct lobbyListAuth_2 *Rbuffer,int divrcnt, int modrcnt) //
{
	int  i, j;

	runQuery("Select * from RoomList"); // 방 목록 가져옴

    for(i = 0;i < divrcnt; i++)
    { // 방 목록 전송 과정 1
        for(j = 0; j < 4; j++)
            {
            fetchRow();
            Rbuffer[i].room[j].rid = atoi(sql_row[0]);
            Rbuffer[i].room[j].ucount = atoi(sql_row[1]);
            strncpy(Rbuffer[i].room[j].hname, sql_row[2], sizeof(Rbuffer[i].room[j].hname));
            }
    }
    for (i = 0; i < modrcnt; i++)  // 방 목록 전송 과정 2 0~3개 남은 방들을 담아 전송
    {
         fetchRow();
         Rbuffer[divrcnt].room[i].rid = atoi(sql_row[0]);
         Rbuffer[divrcnt].room[i].ucount = atoi(sql_row[1]);
         strncpy(Rbuffer[divrcnt].room[i].hname, sql_row[2], sizeof(Rbuffer[divrcnt].room[i].hname));
    }
    Log("bringRoomList function success!");
}

// 방 접속시 호출, 방을 갱신하고 다른 유저의 정보를 불러옴, 방장 + host1,2,3 구조
int enterRoom(int rid, int uid, int fd, int *stats, int *broadcastfd, struct lobbyEnterAuth *RoomUser)
{
    int ucnt, i, j;
    int guest[3] = {0}; // guest 1,2,3의 uid
    char roomHostId[16] = {0}; // 방장의 id를 넣을 임시 버퍼

    memset(query, 0x00, sizeof(query));

    // 해당 방 번호에 있는 유저의 기록을 불러온다
    snprintf(query, 255, "Select * from RoomList where rid = %d",rid);
	i = runQuery(query); // 오류 체크용으로 임시로 박아둠
	fetchRow();
	ucnt = atoi(sql_row[1]); // 방에 있는 유저 수
	RoomUser->ucnt = ucnt; // 방에 있는 유저 수를 담음
    if (i == -1 || ucnt == 4 ) // 일치하는 rid가 없거나 ucnt = 4인 경우
    {
        Log("EnterRoom failed");
        return 0;
    }


	//guest 들의 uid와 방장의 id 조회
    strncpy(roomHostId, sql_row[2], sizeof(roomHostId)); // 방장의 id를 가져옴
    for(i = 0; i < 3; i++) // guest 1,2,3 uid 가져옴
    {
        guest[i] = atoi(sql_row[i+3]);
    }

    for(i = 0; i < 4; i++) // broadcast용 fd 저장
    {
        broadcastfd[i] = atoi(sql_row[i+6]);
    }

    //방장의 정보를 user[0]에 대입
    snprintf(query, 255, "Select * from Userinfo where id = '%s'", roomHostId);
	runQuery(query);
    fetchRow();
    strncpy(RoomUser->user[0].id, sql_row[1], sizeof(RoomUser->user[0].id));
    RoomUser->user[0].slot = 0; // 1번 슬롯 고정이므로
	RoomUser->user[0].stats = stats[0];
    RoomUser->user[0].win = atoi(sql_row[3]);
    RoomUser->user[0].lose = atoi(sql_row[4]);
    RoomUser->user[0].kill = atoi(sql_row[5]);
    RoomUser->user[0].death = atoi(sql_row[6]);

    // 방장을 제외한 guest 1,2,3의 정보를 넣는 과정
    for(i = 0; i < 3; i++)
    {// 처음 빈칸이 나오면 들어올 host의 정보를 삽입하고 다음 빈칸은 그냥 continue
        if(guest[i] == 0) // 이 slot은 비어있음, 접속한 유저를 삽입하고 ucount 갱신
        {
			RoomUser->slot = i+1; // 삽입된 유저의 슬롯 번호
            stats[i+1] = 0; // 유저를 삽입 하였으므로 유저 상태의 값을 변경
            snprintf(query, 255, "update RoomList set ucount = %d,guest%d = %d, guest%dfd = %d where rid = %d", ucnt+1, i+2, uid, i+2, fd, rid);
            runQuery(query);
            i++;
            break;
        }
    }
	for(i; i <3; i++)
	{
		if(guest[i] == 0) // 다음 빈칸이므로 continue
			continue;

		else // 유저 정보를 대입한다, segmentation fault를 막기위해 대입 후 다시 for을 돌려 유저 정보를 다시 대입한다.
		{
			snprintf(query, 255, "Select * from Userinfo where uid = %d", guest[i]);
			fetchRow();
			strncpy(RoomUser->user[1].id, sql_row[1], sizeof(RoomUser->user[1].id));
			RoomUser->user[1].slot = i+1; // i+1번 슬롯에 배치되어 있다.
			RoomUser->user[1].stats = stats[i+1]; // i+1번 상태를 보면 암
			RoomUser->user[1].win = atoi(sql_row[3]);
			RoomUser->user[1].lose = atoi(sql_row[4]);
			RoomUser->user[1].kill = atoi(sql_row[5]);
			RoomUser->user[1].death = atoi(sql_row[6]);
			i++;
			break;
		}
	}
	for(i; i <3; i++)
	{
		if(guest[i] == 0) // 다음 빈칸이므로 continue
			continue;
		else
		{
			snprintf(query, 255, "Select * from Userinfo where uid = %d", guest[i]);
			fetchRow();
			strncpy(RoomUser->user[2].id, sql_row[1], sizeof(RoomUser->user[2].id));
			RoomUser->user[2].slot = i+1; // i+1번 슬롯에 배치되어 있다.
			RoomUser->user[2].stats = stats[i+1]; // i+1번 상태를 보면 암
			RoomUser->user[2].win = atoi(sql_row[3]);
			RoomUser->user[2].lose = atoi(sql_row[4]);
			RoomUser->user[2].kill = atoi(sql_row[5]);
			RoomUser->user[2].death = atoi(sql_row[6]);
		}
	}

	Log("EnterRoom Success!");
	return 1; // 성공적으로 마침
}

// 상태가 변경된 유저에 대한 slot과 stats를 전달하기 위한 함수
int broadcastInRoom(int fd, int rid, int *broadcastfd, struct inRoomStateBroadcast *roomBroad)
{
	int i;
	memset(query, 0x00, sizeof(query));

    // 해당 방 번호에 있는 유저의 기록을 불러온다
    snprintf(query, 255, "Select * from RoomList where rid = %d",rid);
	if (runQuery(query) == -1)
	{
		Log("broadcastInRoom failed");
		return 0;
	}
	fetchRow();

	for(i = 0; i < 4; i++) // broadcast용 fd 저장, 변경된 유저의 슬롯을 저장한다.
    {
        broadcastfd[i] = atoi(sql_row[i+6]);
		if(fd == broadcastfd[i]) // 변경된 유저
		{
			roomBroad->slot = i;
		}
    }

	Log("broadcastInRoom Success!");
	return 1; // 성공적으로 마침
}

void deleteRoom(int rid) // 방 삭제시 list에서 제외한다.
{
	memset(query, 0x00, sizeof(query));

	snprintf(query, 255, "delete from RoomList where rid = %d",rid); // 방 삭제
	runQuery(query);
	Log("deleteRoom success!");
}

 // 게임이 끝나고 유저 정보를 갱신한다
void updateUserinfo(struct profile Upf)
{
	int win, lose, kill, death;

	win = Upf.win;
	lose = Upf.lose;
	kill = Upf.kill;
	death = Upf.death;
	char id[16] = {0};

	memcpy(id, Upf.id, sizeof(id));

	snprintf(query, 255, "update Userinfo set win = %d, lose = %d, u_kill = %d, u_death = %d where id = '%s'", win, lose, kill, death, id);
    runQuery(query);
}

void DB_errMsg(char *errMsg)
{
        printf("%s\n",errMsg);
        fprintf(stderr, "Error Meassage : %s \n", mysql_error(conn));
}

