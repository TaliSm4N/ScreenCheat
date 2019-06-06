#include <mysql/mysql.h>

int     runQuery(char *); // 쿼리 실행
int     fetchRow(void); //로우 조회 , sql_result에 query로 받은 row들을 실행때마다 차례대로 받아옴
int     connectDB(void);
void    closeDB(void);

int     addCount(char *); //유저나 방의 개수를 호출마다 하나씩 추가한다.
int 	inquiryRcount(); // Roomcount 조회
char*   convertUid(int); // sid를 통해 user id를 얻어온다
void	bringUserinfo(int, struct user *);

// 로그인
int 	joinMembership(char *, char *); // 회원가입, 유저의 수도 갱신된다.
int		compareID(char *, char *,struct user *,int *); // ID, PWD를 비교한다. TRUE면 일치, FALSE면 일치하는 정보가 없다(틀리는것도 해당)

// 방
void	createRoom(int, int, struct room *); // 방 생성시 list에 추가(방 번호, 호스트 이름,호스트 fd)
void	bringRoomList(struct lobbyListAuth *[],int, int); // 방 목록을 가져온다, 방 수를 리턴
int 	enterRoom(int, int, int, int*, int *, struct lobbyEnterAuth *); // 방 접속
void	updateRoom(); // 생성된 방에 참여/퇴장시 인원 변경
void	deleteRoom(); // 방 삭제시 rcnt를 낮추고 list에서 제외한다.

void    DB_errMsg(char *); // 에러메시지 출력
