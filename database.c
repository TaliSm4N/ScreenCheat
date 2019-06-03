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
  /*  runQuery("Select * from count");
	fetchRow();
    ucnt = atoi(sql_row[0]) + 1; // 유저 수 하나 추가
    snprintf(query, 255, "update count SET ucnt = %d",ucnt);
    runQuery(query); // 갱신 */

	if (sql_result)
                mysql_free_result(sql_result);

    printf("join Success\n");

    return 1;
}

int compareID(char *id, char *pwd,struct profile *Upf,int * sid) // ID, PWD를 비교한다. TRUE면 일치, FALSE면 일치하는 정보가 없다(틀리는것도 해당)
{ // ucnt는 단순히 반복문 수행을 위해서 유저 수를 체크하는 변수
    runQuery("Select * from count");
	fetchRow();
    int ucnt = atoi(sql_row[0]);
    printf("ucnt = %d \n", ucnt);
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
void createRoom(int sid)
{
    int rcnt;
	char query[255] = {0}; // 임시로 담을 버퍼
	char idBuf[16] = {0};
	/*
	runQuery("Select * from count");
	fetchRow();
    rcnt = atoi(sql_row[0]) + 1; // 방 수 하나 추가
    snprintf(query, 255, "update count SET rcnt = %d",rcnt);
    runQuery(query); // 갱신 */
	strncpy(idBuf, convertSid(sid), sizeof(idBuf)); 
	snprintf(query, 255, "insert into RoomList (rid, hname)values('%d', '%s')", addCount("rcnt"), idBuf);
	runQuery(query);


	if (sql_result)
                mysql_free_result(sql_result);

    printf("Create Room Success\n");

}

// 방 목록을 가져온다.
void bringRoomList()
{
	int i;
	runQuery("Select * from RoomList"); // 방 목록 가져옴

	//for(i = 0; i < rcnt; i++)
		//fetchRow();

}


void DB_errMsg(char *errMsg)
{
        printf("%s\n",errMsg);
        fprintf(stderr, "Error Meassage : %s \n", mysql_error(conn));
}

