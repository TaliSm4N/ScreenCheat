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

//유저 회원가입을 시킴, sid는 자동으로 0부터 1씩 추가된다.
void joinMembership(char *id, char *pwd)
{
	char query[255] = {0}; // 임시로 담을 버퍼
	snprintf(query, 255,"insert into Userinfo (id, pwd, win, lose, u_kill, u_death) values('%s', '%s', '0', '0', '0', '0')", id, pwd);
	printf("database");
	runQuery(query);
}

void compareID(char *id, char *pwd,struct profile *Upf,int * sid) // ID, PWD를 비교한다. TRUE면 일치, FALSE면 일치하는 정보가 없다(틀리는것도 해당)
{
	runQuery("Select * from Userinfo"); // 비교대상을 가져옴

	while(fetchRow()) // 검사 진행
	{
		if ((!strcmp(sql_row[1],id))&&(!strcmp(sql_row[2],pwd))) { // 유저의 id,pwd가 일치하는 상황
			//유저의 정보를 보내기위해 불러온 값을 저장한다.
			*sid = atoi(sql_row[0]);
			strncpy(Upf->id, sql_row[1], sizeof(Upf->id));
			Upf->win = atoi(sql_row[3]);
			Upf->lose = atoi(sql_row[4]);
			Upf->kill = atoi(sql_row[5]);
			Upf->death = atoi(sql_row[6]);

		//	return TRUE;
		}
		else
			continue;
	}
	*sid = -1; // login fail
//	return FALSE;
}

// 방 생성시 rcnt와 list 추가
void createRoom(struct room Room)
{
	char query[255] = {0}; // 임시로 담을 버퍼
	snprintf(query, 255,"insert into RoomList values('%d', '%d', '%s')", Room.rid, Room.ucount, Room.hname);
	runQuery(query);
	//rcnt++; // 방 개수 하나 증가
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

