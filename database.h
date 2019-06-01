#include <mysql/mysql.h>

int     runQuery(char *query); // 쿼리 실행
int     fetchRow(void); //로우 조회 , sql_result에 query로 받은 row들을 실행때마다 차례대로 받아옴
int     connectDB(void);
void    closeDB(void);
// 로그인
void	joinMembership(char *id, char *pwd); // 회원가입, 유저의 수도 갱신된다.
int	compareID(char *id, char *pwd,struct profile *Upf,int *sid); // ID, PWD를 비교한다. TRUE면 일치, FALSE면 일치하는 정보가 없다(틀리는것도 해당)
// 방
void	createRoom(struct room Room); // 방 생성시 rcnt와 list 추가
void	bringRoomList(); // 방 목록을 가져온다.
void	updateRoom(); // 생성된 방에 참여/퇴장시 인원 변경
void	deleteRoom(); // 방 삭제시 rcnt를 낮추고 list에서 제외한다.

void    DB_errMsg(char *errMsg); // 에러메시지 출력
