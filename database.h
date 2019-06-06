#include <mysql/mysql.h>

int     runQuery(char *); // ���� ����
int     fetchRow(void); //�ο� ��ȸ , sql_result�� query�� ���� row���� ���ට���� ���ʴ�� �޾ƿ�
int     connectDB(void);
void    closeDB(void);

int     addCount(char *); //������ ���� ������ ȣ�⸶�� �ϳ��� �߰��Ѵ�.
int 	inquiryRcount(); // Roomcount ��ȸ
char*   convertUid(int); // sid�� ���� user id�� ���´�
void	bringUserinfo(int, struct user *);

// �α���
int 	joinMembership(char *, char *); // ȸ������, ������ ���� ���ŵȴ�.
int		compareID(char *, char *,struct user *,int *); // ID, PWD�� ���Ѵ�. TRUE�� ��ġ, FALSE�� ��ġ�ϴ� ������ ����(Ʋ���°͵� �ش�)

// ��
void	createRoom(int, int, struct room *); // �� ������ list�� �߰�(�� ��ȣ, ȣ��Ʈ �̸�,ȣ��Ʈ fd)
void	bringRoomList(struct lobbyListAuth *[],int, int); // �� ����� �����´�, �� ���� ����
int 	enterRoom(int, int, int, int*, int *, struct lobbyEnterAuth *); // �� ����
void	updateRoom(); // ������ �濡 ����/����� �ο� ����
void	deleteRoom(); // �� ������ rcnt�� ���߰� list���� �����Ѵ�.

void    DB_errMsg(char *); // �����޽��� ���
