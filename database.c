#include "server.h"

MYSQL_RES *sql_result;
MYSQL_ROW sql_row;
MYSQL *conn;

// ���� ����
int runQuery(char *query)
{
        if(mysql_query(conn, query)) {
                DB_errMsg("MySQL Query Excute failed");
                return -1;
        }

        sql_result = mysql_store_result(conn);
        return 0;
}

//�ο� ��ȸ , sql_result�� query�� ���� row���� ���ට���� ���ʴ�� �޾ƿ�
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

// ��� ����
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

// ��� ���� ����
void closeDB(void)
{
        if (sql_result)
                mysql_free_result(sql_result);
        mysql_close(conn);
}

//���� ȸ�������� ��Ŵ, sid�� �ڵ����� 0���� 1�� �߰��ȴ�.
void joinMembership(char *id, char *pwd)
{
	char query[255] = {0}; // �ӽ÷� ���� ����
	snprintf(query, 255,"insert into Userinfo (id, pwd, win, lose, u_kill, u_death) values('%s', '%s', '0', '0', '0', '0')", id, pwd);
	printf("database");
	runQuery(query);
}

void compareID(char *id, char *pwd,struct profile *Upf,int * sid) // ID, PWD�� ���Ѵ�. TRUE�� ��ġ, FALSE�� ��ġ�ϴ� ������ ����(Ʋ���°͵� �ش�)
{
	runQuery("Select * from Userinfo"); // �񱳴���� ������

	while(fetchRow()) // �˻� ����
	{
		if ((!strcmp(sql_row[1],id))&&(!strcmp(sql_row[2],pwd))) { // ������ id,pwd�� ��ġ�ϴ� ��Ȳ
			//������ ������ ���������� �ҷ��� ���� �����Ѵ�.
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

// �� ������ rcnt�� list �߰�
void createRoom(struct room Room)
{
	char query[255] = {0}; // �ӽ÷� ���� ����
	snprintf(query, 255,"insert into RoomList values('%d', '%d', '%s')", Room.rid, Room.ucount, Room.hname);
	runQuery(query);
	//rcnt++; // �� ���� �ϳ� ����
}

// �� ����� �����´�.
void bringRoomList()
{
	int i;
	runQuery("Select * from RoomList"); // �� ��� ������

	//for(i = 0; i < rcnt; i++)
		//fetchRow();

}


void DB_errMsg(char *errMsg)
{
        printf("%s\n",errMsg);
        fprintf(stderr, "Error Meassage : %s \n", mysql_error(conn));
}

