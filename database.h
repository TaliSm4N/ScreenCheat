#include <mysql/mysql.h>

int     runQuery(char *query); // ���� ����
int     fetchRow(void); //�ο� ��ȸ , sql_result�� query�� ���� row���� ���ට���� ���ʴ�� �޾ƿ�
int     connectDB(void);
void    closeDB(void);
// �α���
void	joinMembership(char *id, char *pwd); // ȸ������, ������ ���� ����
int	compareID(char *id, char *pwd,struct profile *Upf,int *sid); // ID, PWD�� ���Ѵ�. TRUE�� ��ġ, FALSE�� ��ġ�ϴ� ������ ����(Ʋ���°͵� �ش�)
// ��
void	createRoom(struct room Room); // �� ������ rcnt�� list �߰�
void	bringRoomList(); // �� ����� �����´�.
void	updateRoom(); // ������ �濡 ����/����� �ο� ����
void	deleteRoom(); // �� ������ rcnt�� ���߰� list���� �����Ѵ�.

void    DB_errMsg(char *errMsg); // �����޽��� ���
