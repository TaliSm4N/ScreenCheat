#include <mysql/mysql.h>

int     runQuery(char *); // ���� ����
int     fetchRow(void); //�ο� ��ȸ , sql_result�� query�� ���� row���� ���ට���� ���ʴ�� �޾ƿ�
int     connectDB(void);
void    closeDB(void);

int 	inquiryCount(char *); // ����, �� �� ��ȸ
int		inquiryHostfd(int); // ȣ��Ʈ�� fd�� ������
char*   convertUid(int); // uid�� ���� user id�� ���´�
void	bringUserinfo(int, struct user *); // uid�� �´� ������ ������ ������
int     checkRoomid(void); // rid�� �߰��� ��� �ִ��� üũ�Ѵ�.

// �α���
int 	joinMembership(char *, char *); //���� ȸ�������� ��Ŵ, uid�� 1�� �߰��ȴ�
int		compareID(char *, char *,struct profile *,int *);// ID, PWD�� ���Ѵ�. TRUE�� ��ġ, FALSE�� ��ġ�ϴ� ������ ����(Ʋ���°͵� �ش�)

// ����
void	createRoom(int, int, struct lobbyCreateAuth *); // �� ������ list�� �߰�(�� ��ȣ, ȣ��Ʈ �̸�,ȣ��Ʈ fd)
void    exitRoom(int, int, int *); // �� ����� �ο� update, ������ �ƴ� �ٸ� �ο��� ���ؼ��� update�� ����ȴ�
void	bringRoomList(struct lobbyListAuth_2 *,int, int); // �� ����� �����´�, �� ���� ����
int 	enterRoom(int, int, int, int*, int *, struct lobbyEnterAuth *); // �� ����

// ��
int		broadcastInRoom(int ,int, int*, struct inRoomStateBroadcast *); // ��ε�ĳ����
void	deleteRoom(int); // �� ������ list���� �����Ѵ�.

// �ΰ���

// ������ ���� ����
void	updateUserinfo(struct profile); // ������ ������ ���� ������ �����Ѵ�

void    DB_errMsg(char *); // �����޽��� ���



