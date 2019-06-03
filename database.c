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

//������ ���� ������ ȣ�⸶�� �ϳ��� �߰��Ѵ�.
int addCount(char *cntType)
{
	int cnt;
	char query[255] = {0}; // �ӽ÷� ���� ����
	
	runQuery("Select * from count");
	fetchRow();
    cnt = atoi(sql_row[0]) + 1; // ���� �� �ϳ� �߰�
    snprintf(query, 255, "update count SET %s = %d", cntType, cnt);
    runQuery(query); // ����
	
	return cnt;
}

// sid�� ����id�� ã�ƿ´�.
char* convertSid(int sid)
{
	runQuery("Select * from count");
	fetchRow();
    int ucnt = atoi(sql_row[0]);
	printf("ucnt = %d \n", ucnt);
	runQuery("Select * from Userinfo"); // �񱳴���� ������
	
	for(int i=0; i < ucnt; i++)
	{
	    fetchRow(); // �˻� ����
		if (sid == atoi(sql_row[0])) 
			return sql_row[1];
	}
	return NULL; // ����id�� ã�� ����
}

//���� ȸ�������� ��Ŵ, sid�� �ڵ����� 0���� 1�� �߰��ȴ�, ������ ���� �þ
int joinMembership(char *id, char *pwd)
{

    int ucnt;
	char query[255] = {0}; // �ӽ÷� ���� ����

    memset(query, 0x00, sizeof(query));
	snprintf(query, 255,"insert into Userinfo (id, pwd) values('%s', '%s')", id, pwd);
	if (runQuery(query) == -1) // ���̵� �ߺ�
        return 0;

	addCount("ucnt");
  /*  runQuery("Select * from count");
	fetchRow();
    ucnt = atoi(sql_row[0]) + 1; // ���� �� �ϳ� �߰�
    snprintf(query, 255, "update count SET ucnt = %d",ucnt);
    runQuery(query); // ���� */

	if (sql_result)
                mysql_free_result(sql_result);

    printf("join Success\n");

    return 1;
}

int compareID(char *id, char *pwd,struct profile *Upf,int * sid) // ID, PWD�� ���Ѵ�. TRUE�� ��ġ, FALSE�� ��ġ�ϴ� ������ ����(Ʋ���°͵� �ش�)
{ // ucnt�� �ܼ��� �ݺ��� ������ ���ؼ� ���� ���� üũ�ϴ� ����
    runQuery("Select * from count");
	fetchRow();
    int ucnt = atoi(sql_row[0]);
    printf("ucnt = %d \n", ucnt);
	runQuery("Select * from Userinfo"); // �񱳴���� ������


	for(int i=0; i < ucnt; i++)
	{
	    fetchRow(); // �˻� ����
		if ((!strcmp(sql_row[1],id))&&(!strcmp(sql_row[2],pwd))) { // ������ id,pwd�� ��ġ�ϴ� ��Ȳ
			//������ ������ ���������� �ҷ��� ���� �����Ѵ�.
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

// �� ������ list�� �߰�(�� ��ȣ, ȣ��Ʈ �̸�)
void createRoom(int sid)
{
    int rcnt;
	char query[255] = {0}; // �ӽ÷� ���� ����
	char idBuf[16] = {0};
	/*
	runQuery("Select * from count");
	fetchRow();
    rcnt = atoi(sql_row[0]) + 1; // �� �� �ϳ� �߰�
    snprintf(query, 255, "update count SET rcnt = %d",rcnt);
    runQuery(query); // ���� */
	strncpy(idBuf, convertSid(sid), sizeof(idBuf)); 
	snprintf(query, 255, "insert into RoomList (rid, hname)values('%d', '%s')", addCount("rcnt"), idBuf);
	runQuery(query);


	if (sql_result)
                mysql_free_result(sql_result);

    printf("Create Room Success\n");

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

