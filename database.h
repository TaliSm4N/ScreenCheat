#include <mysql/mysql.h>

int     runQuery(char *); // 쿼리 실행
int     fetchRow(void); //로우 조회 , sql_result에 query로 받은 row들을 실행때마다 차례대로 받아옴
int     connectDB(void);
void    closeDB(void);

int 	inquiryCount(char *); // 유저, 방 수 조회
int		inquiryHostfd(int); // 호스트의 fd를 가져옴
char*   convertUid(int); // uid를 통해 user id를 얻어온다
void	bringUserinfo(int, struct user *); // uid에 맞는 유저의 정보를 가져옴
int     checkRoomid(void); // rid가 중간에 빈게 있는지 체크한다.

// 로그인
int 	joinMembership(char *, char *); //유저 회원가입을 시킴, uid는 1씩 추가된다
int		compareID(char *, char *,struct profile *,int *);// ID, PWD를 비교한다. TRUE면 일치, FALSE면 일치하는 정보가 없다(틀리는것도 해당)

// 대기실
void	createRoom(int, int, struct lobbyCreateAuth *); // 방 생성시 list에 추가(방 번호, 호스트 이름,호스트 fd)
void    exitRoom(int, int, int *); // 방 퇴장시 인원 update, 방장이 아닌 다른 인원에 대해서만 update가 진행된다
void	bringRoomList(struct lobbyListAuth_2 *,int, int); // 방 목록을 가져온다, 방 수를 리턴
int 	enterRoom(int, int, int, int*, int *, struct lobbyEnterAuth *); // 방 접속

// 방
int		broadcastInRoom(int ,int, int*, struct inRoomStateBroadcast *); // 브로드캐스팅
void	deleteRoom(int); // 방 삭제시 list에서 제외한다.

// 인게임

// 게임이 끝난 이후
void	updateUserinfo(struct profile); // 게임이 끝나고 유저 정보를 갱신한다

void    DB_errMsg(char *); // 에러메시지 출력



