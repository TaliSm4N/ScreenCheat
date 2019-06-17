#include "server.h"

MYSQL_RES *sql_result;
MYSQL_ROW sql_row;
MYSQL *conn;

char query[255] = {0}; // �ӽ÷� ���� ����

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

// ����, �� �� ��ȸ
int inquiryCount(char *cntTable)
{
	int cnt;

    memset(query, 0x00, sizeof(query));
	snprintf(query, 255, "select count(*) from %s", cntTable);
	runQuery(query);
	fetchRow();
    cnt = atoi(sql_row[0]); // ����, �� �� ��ȸ

	return cnt;
}

int inquiryHostfd(int rid)
{
    int i;
    memset(query, 0x00, sizeof(query));

    snprintf(query, 255, "Select hostfd from RoomList where rid = %d",rid);
	runQuery(query); // ���� üũ������ �ӽ÷� �ھƵ�
	fetchRow();

	Log("inquiryHostfd success!");
    return atoi(sql_row[0]);
}

// sid�� ����id�� ã�ƿ´�.
char* convertUid(int uid)
{
    int ucnt = inquiryCount("Userinfo"); // ���� �� ��ȸ
	printf("ucnt = %d \n", ucnt);
	runQuery("Select * from Userinfo"); // �񱳴���� ������

	for(int i=0; i < ucnt; i++)
	{
	    fetchRow(); // �˻� ����
		if (uid == atoi(sql_row[0]))
			return sql_row[1];
	}
	return NULL; // ����id�� ã�� ����
}

// uid�� �´� ������ ������ ������
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

// rid�� �߰��� ��� �ִ��� üũ�Ѵ�.
int checkRoomid(void)
{
	int i = 1, rcnt = inquiryCount("RoomList");
	Log("Check Roomid");
	memset(query, 0x00, sizeof(query));
	runQuery("select rid from RoomList");

	for(i = 1; i <= rcnt; i++)
	{
		fetchRow();
		if (i != atoi(sql_row[0])) // �߰� ���ڰ� �� ���
		{
		    return i;
		}

	}

	return i; // ������ ��ģ��
}

//���� ȸ�������� ��Ŵ, uid�� 1�� �߰��ȴ�
int joinMembership(char *id, char *pwd)
{
    int ucnt = inquiryCount("Userinfo") + 1; // uid = ucnt + 1
    LogNum("ucnt + 1",ucnt);
    memset(query, 0x00, sizeof(query));
	snprintf(query, 255,"insert into Userinfo (uid,id, pwd) values('%d' ,'%s', '%s')", ucnt, id, pwd);
	if (runQuery(query) == -1) // ���̵� �ߺ�
        return 0;

	if (sql_result)
                mysql_free_result(sql_result);

    Log("join Success");

    return 1;
}

int compareID(char *id, char *pwd, struct profile *Upf, int * uid) // ID, PWD�� ���Ѵ�. TRUE�� ��ġ, FALSE�� ��ġ�ϴ� ������ ����(Ʋ���°͵� �ش�)
{
    int ucnt = inquiryCount("Userinfo"); // ���� �� ��ȸ

	runQuery("Select * from Userinfo"); // �񱳴���� ������


	for(int i=0; i < ucnt; i++)
	{
	    fetchRow(); // �˻� ����
		if ((!strcmp(sql_row[1],id))&&(!strcmp(sql_row[2],pwd))) { // ������ id,pwd�� ��ġ�ϴ� ��Ȳ
			//������ ������ ���������� �ҷ��� ���� �����Ѵ�.
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

// �� ������ list�� �߰�(�� ��ȣ, ȣ��Ʈ �̸�,ȣ��Ʈ fd)
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

// �� ����� �ο� update, ������ �ƴ� �ٸ� �ο��� ���ؼ��� update�� ����ȴ�
void exitRoom(int fd, int rid, int *broadcastfd)
{
    int i, ucnt;

    // �ش� rid�� ucnt �� �������� ����
    snprintf(query, 255, "Select ucount from RoomList where rid = %d", rid);
	runQuery(query);
	fetchRow();
	ucnt = atoi(sql_row[1]) - 1; // �濡 �ִ� ���� ������ 1 ��

    memset(query, 0x00, sizeof(query));
    for(i = 1; i < 4; i++)
    {
        if(broadcastfd[i] == fd) // ���� ���� ��ġ
        {
            LogNum("exitRoom fd", fd);
            snprintf(query, 255, "update RoomList set ucount = %d, guest%d = 0, guest%dfd = 0 where rid = %d", ucnt, i+1, i+1, rid);
            runQuery(query);
        }
    }
    Log("exitRoom update Success");
}

// �� ����� �����´�.
void bringRoomList(struct lobbyListAuth_2 *Rbuffer,int divrcnt, int modrcnt) //
{
	int  i, j;

	runQuery("Select * from RoomList"); // �� ��� ������

    for(i = 0;i < divrcnt; i++)
    { // �� ��� ���� ���� 1
        for(j = 0; j < 4; j++)
            {
            fetchRow();
            Rbuffer[i].room[j].rid = atoi(sql_row[0]);
            Rbuffer[i].room[j].ucount = atoi(sql_row[1]);
            strncpy(Rbuffer[i].room[j].hname, sql_row[2], sizeof(Rbuffer[i].room[j].hname));
            }
    }
    for (i = 0; i < modrcnt; i++)  // �� ��� ���� ���� 2 0~3�� ���� ����� ��� ����
    {
         fetchRow();
         Rbuffer[divrcnt].room[i].rid = atoi(sql_row[0]);
         Rbuffer[divrcnt].room[i].ucount = atoi(sql_row[1]);
         strncpy(Rbuffer[divrcnt].room[i].hname, sql_row[2], sizeof(Rbuffer[divrcnt].room[i].hname));
    }
    Log("bringRoomList function success!");
}

// �� ���ӽ� ȣ��, ���� �����ϰ� �ٸ� ������ ������ �ҷ���, ���� + host1,2,3 ����
int enterRoom(int rid, int uid, int fd, int *stats, int *broadcastfd, struct lobbyEnterAuth *RoomUser)
{
    int ucnt, i, j;
    int guest[3] = {0}; // guest 1,2,3�� uid
    char roomHostId[16] = {0}; // ������ id�� ���� �ӽ� ����

    memset(query, 0x00, sizeof(query));

    // �ش� �� ��ȣ�� �ִ� ������ ����� �ҷ��´�
    snprintf(query, 255, "Select * from RoomList where rid = %d",rid);
	i = runQuery(query); // ���� üũ������ �ӽ÷� �ھƵ�
	fetchRow();
	ucnt = atoi(sql_row[1]); // �濡 �ִ� ���� ��
	RoomUser->ucnt = ucnt; // �濡 �ִ� ���� ���� ����
    if (i == -1 || ucnt == 4 ) // ��ġ�ϴ� rid�� ���ų� ucnt = 4�� ���
    {
        Log("EnterRoom failed");
        return 0;
    }


	//guest ���� uid�� ������ id ��ȸ
    strncpy(roomHostId, sql_row[2], sizeof(roomHostId)); // ������ id�� ������
    for(i = 0; i < 3; i++) // guest 1,2,3 uid ������
    {
        guest[i] = atoi(sql_row[i+3]);
    }

    for(i = 0; i < 4; i++) // broadcast�� fd ����
    {
        broadcastfd[i] = atoi(sql_row[i+6]);
    }

    //������ ������ user[0]�� ����
    snprintf(query, 255, "Select * from Userinfo where id = '%s'", roomHostId);
	runQuery(query);
    fetchRow();
    strncpy(RoomUser->user[0].id, sql_row[1], sizeof(RoomUser->user[0].id));
    RoomUser->user[0].slot = 0; // 1�� ���� �����̹Ƿ�
	RoomUser->user[0].stats = stats[0];
    RoomUser->user[0].win = atoi(sql_row[3]);
    RoomUser->user[0].lose = atoi(sql_row[4]);
    RoomUser->user[0].kill = atoi(sql_row[5]);
    RoomUser->user[0].death = atoi(sql_row[6]);

    // ������ ������ guest 1,2,3�� ������ �ִ� ����
    for(i = 0; i < 3; i++)
    {// ó�� ��ĭ�� ������ ���� host�� ������ �����ϰ� ���� ��ĭ�� �׳� continue
        if(guest[i] == 0) // �� slot�� �������, ������ ������ �����ϰ� ucount ����
        {
			RoomUser->slot = i+1; // ���Ե� ������ ���� ��ȣ
            stats[i+1] = 0; // ������ ���� �Ͽ����Ƿ� ���� ������ ���� ����
            snprintf(query, 255, "update RoomList set ucount = %d,guest%d = %d, guest%dfd = %d where rid = %d", ucnt+1, i+2, uid, i+2, fd, rid);
            runQuery(query);
            i++;
            break;
        }
    }
	for(i; i <3; i++)
	{
		if(guest[i] == 0) // ���� ��ĭ�̹Ƿ� continue
			continue;

		else // ���� ������ �����Ѵ�, segmentation fault�� �������� ���� �� �ٽ� for�� ���� ���� ������ �ٽ� �����Ѵ�.
		{
			snprintf(query, 255, "Select * from Userinfo where uid = %d", guest[i]);
			fetchRow();
			strncpy(RoomUser->user[1].id, sql_row[1], sizeof(RoomUser->user[1].id));
			RoomUser->user[1].slot = i+1; // i+1�� ���Կ� ��ġ�Ǿ� �ִ�.
			RoomUser->user[1].stats = stats[i+1]; // i+1�� ���¸� ���� ��
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
		if(guest[i] == 0) // ���� ��ĭ�̹Ƿ� continue
			continue;
		else
		{
			snprintf(query, 255, "Select * from Userinfo where uid = %d", guest[i]);
			fetchRow();
			strncpy(RoomUser->user[2].id, sql_row[1], sizeof(RoomUser->user[2].id));
			RoomUser->user[2].slot = i+1; // i+1�� ���Կ� ��ġ�Ǿ� �ִ�.
			RoomUser->user[2].stats = stats[i+1]; // i+1�� ���¸� ���� ��
			RoomUser->user[2].win = atoi(sql_row[3]);
			RoomUser->user[2].lose = atoi(sql_row[4]);
			RoomUser->user[2].kill = atoi(sql_row[5]);
			RoomUser->user[2].death = atoi(sql_row[6]);
		}
	}

	Log("EnterRoom Success!");
	return 1; // ���������� ��ħ
}

// ���°� ����� ������ ���� slot�� stats�� �����ϱ� ���� �Լ�
int broadcastInRoom(int fd, int rid, int *broadcastfd, struct inRoomStateBroadcast *roomBroad)
{
	int i;
	memset(query, 0x00, sizeof(query));

    // �ش� �� ��ȣ�� �ִ� ������ ����� �ҷ��´�
    snprintf(query, 255, "Select * from RoomList where rid = %d",rid);
	if (runQuery(query) == -1)
	{
		Log("broadcastInRoom failed");
		return 0;
	}
	fetchRow();

	for(i = 0; i < 4; i++) // broadcast�� fd ����, ����� ������ ������ �����Ѵ�.
    {
        broadcastfd[i] = atoi(sql_row[i+6]);
		if(fd == broadcastfd[i]) // ����� ����
		{
			roomBroad->slot = i;
		}
    }

	Log("broadcastInRoom Success!");
	return 1; // ���������� ��ħ
}

void deleteRoom(int rid) // �� ������ list���� �����Ѵ�.
{
	memset(query, 0x00, sizeof(query));

	snprintf(query, 255, "delete from RoomList where rid = %d",rid); // �� ����
	runQuery(query);
	Log("deleteRoom success!");
}

 // ������ ������ ���� ������ �����Ѵ�
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

