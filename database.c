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

// Roomcount ��ȸ
int inquiryRcount()
{
    runQuery("Select * from count");
    fetchRow();
    LogNum("RoomCount",atoi(sql_row[1]));
    return atoi(sql_row[1]);
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

	if (sql_result)
                mysql_free_result(sql_result);

    Log("join Success");

    return 1;
}

int compareID(char *id, char *pwd,struct profile *Upf,int * sid) // ID, PWD�� ���Ѵ�. TRUE�� ��ġ, FALSE�� ��ġ�ϴ� ������ ����(Ʋ���°͵� �ش�)
{ // ucnt�� �ܼ��� �ݺ��� ������ ���ؼ� ���� ���� üũ�ϴ� ����
    runQuery("Select * from count");
	fetchRow();
    int ucnt = atoi(sql_row[0]);
    LogNum("ucnt",ucnt);
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
void createRoom(int sid, struct room *Rinfo)
{
    int rcnt;
	char query[255] = {0}; // �ӽ÷� ���� ����
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

// �� ����� �����´�.
void bringRoomList(struct lobbyListAuth *Rbuffer[],int divrcnt, int modrcnt) //
{
	int  i = 0, j; //,rcnt;

//    rcnt = inquiryRcount();

	runQuery("Select * from RoomList"); // �� ��� ������

   for(i = 0;i < divrcnt; i++)
    { // �� ��� ���� ���� 1
        fetchRow();
        for(j = 0; j < 4; j++)
            {
            Rbuffer[i]->room[j].rid = atoi(sql_row[0]);
            Rbuffer[i]->room[j].ucount = atoi(sql_row[1]);
            strncpy(Rbuffer[i]->room[j].hname, sql_row[2], sizeof(Rbuffer[i]->room[j].hname));
            }
    }
    for (i = 0; i < modrcnt; i++)  // �� ��� ���� ���� 2 0~3�� ���� ����� ��� ����
    {
         fetchRow();
         Rbuffer[divrcnt]->room[i].rid = atoi(sql_row[0]);
         Rbuffer[divrcnt]->room[i].ucount = atoi(sql_row[1]);
         strncpy(Rbuffer[divrcnt]->room[i].hname, sql_row[2], sizeof(Rbuffer[divrcnt]->room[i].hname));
    }

}

// �� ���ӽ� ȣ��, ���� �����ϰ� �ٸ� ������ ������ �ҷ���, ���� + host1,2,3 ����
int enterRoom(int rid, int sid, struct lobbyEnterAuth *RoomUser)
{
    int ucnt, i, j;
    char roomHostId[16] = {0}; // ������ id�� ���� �ӽ� ����
    char query[255] = {0}; // query ����� �ӽ� ����
    int host[3] = {0}; // host 1,2,3�� sid�� ���� �ӽ� ����

    // �ش� �� ��ȣ�� �ִ� ������ ����� �ҷ��´�
    snprintf(query, 255, "Select * from RoomList where rid = %d",rid);
    if (runQuery(query) == -1)
    {
        Log("EnterRoom failed");
        return 0; // ��ġ�ϴ� rid�� ����
    }
    fetchRow();

	//host ���� sid�� ������ id, �濡 �ִ� ���� �� ��ȸ
    ucnt = atoi(sql_row[1]); // �濡 �ִ� ���� ��
    strncpy(roomHostId, sql_row[2], sizeof(roomHostId)); // ������ id�� ������
    for(i = 0; i < 3; i++) // host 1,2,3 sid ������
    {
        host[i] = atoi(sql_row[i+3]);
    }
    //������ ������ user[0]�� ����
    snprintf(query, 255, "Select * from Userinfo where id = '%s'", roomHostId);
    fetchRow();
    strncpy(RoomUser->user[0].id, sql_row[1], sizeof(RoomUser->user[0].id));
    RoomUser->user[0].slot = 0; // ���Կ� ��ġ�Ǿ���.
    RoomUser->user[0].win = atoi(sql_row[3]);
    RoomUser->user[0].lose = atoi(sql_row[4]);
    RoomUser->user[0].kill = atoi(sql_row[5]);
    RoomUser->user[0].death = atoi(sql_row[6]);

    // ������ ������ host 1,2,3�� ������ �ִ� ����
    for(i = 0; i < 3; i++)
    {// ó�� ��ĭ�� ������ ���� host�� ������ �����ϰ� ���� ��ĭ�� �׳� continue
        if(host[i] == 0) // �� slot�� �������, ������ ������ �����Ѵ�.
        {
            snprintf(query, 255, "update RoomList set host%d = %d where rid = %d", i+2, sid, rid);
            runQuery(query);
            break;
        }
    }
	for(i = 0; i <3; i++)
	{
		if(host[i] == 0) // ���� ��ĭ�̹Ƿ� continue
			continue;

		else // ���� ������ �����Ѵ�, segmentation fault�� �������� ���� �� �ٽ� for�� ���� ���� ������ �ٽ� �����Ѵ�.
		{
			snprintf(query, 255, "Select * from Userinfo where sid = %d", host[i]);
			fetchRow();
			strncpy(RoomUser->user[1].id, sql_row[1], sizeof(RoomUser->user[1].id));
			RoomUser->user[1].slot = 0; // ���Կ� ��ġ�Ǿ���.
			RoomUser->user[1].win = atoi(sql_row[3]);
			RoomUser->user[1].lose = atoi(sql_row[4]);
			RoomUser->user[1].kill = atoi(sql_row[5]);
			RoomUser->user[1].death = atoi(sql_row[6]);
			break;
		}
	}
	for(i; i <3; i++)
	{
		if(host[i] == 0) // ���� ��ĭ�̹Ƿ� continue
			continue;
		else
		{
			snprintf(query, 255, "Select * from Userinfo where sid = %d", host[i]);
			fetchRow();
			strncpy(RoomUser->user[2].id, sql_row[1], sizeof(RoomUser->user[2].id));
			RoomUser->user[2].slot = 0; // ���Կ� ��ġ�Ǿ���.
			RoomUser->user[2].win = atoi(sql_row[3]);
			RoomUser->user[2].lose = atoi(sql_row[4]);
			RoomUser->user[2].kill = atoi(sql_row[5]);
			RoomUser->user[2].death = atoi(sql_row[6]);
		}
	}

    // ������ ���� ucount ���� ����, host ����
    snprintf(query, 255, "update RoomList set ucount = %d where rid = %d", ucnt+1, rid);
    runQuery(query);

	Log("EnterRoom Success!");
	return 1; // ���������� ��ħ
}

void DB_errMsg(char *errMsg)
{
        printf("%s\n",errMsg);
        fprintf(stderr, "Error Meassage : %s \n", mysql_error(conn));
}

