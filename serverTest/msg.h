#ifndef MSG_H
#define MSG_H

//profile에 대한 구조체
struct profile{
	char id[16];
	int win;
	int lose;
	int kill;
	int death;
};

//user에 대한 구조체
struct user{
	char id[16];
	int slot; // n번 슬롯
	int stats; // 상태
	int win;
	int lose;
	int kill;
	int death;
};

//room에 대한 구조체, roomList 요청 시 전송
struct room //[4]
{
	int rid;
	int ucount;
	char hname[16];
};

#ifndef LOGIN
#define LOGIN

//로그인 클라이언트의 메시지 원형 40byte
struct loginMsg
{
	int msg_code;

	char data[36];
};

//로그인 요청
struct loginRequest
{
	int msg_code;

	char id[16];
	char pwd[20];
};

//로그인 승인
struct loginAuth
{
	int msg_code;

	int uid;
	struct profile profile;
};

#endif

#ifndef LOBBY
#define LOBBY


//대기실 클라이언트의 메시지 원형 12byte
struct lobbyMsg
{
	int msg_code;

	char data[8];
};

//대기실 요청 , 방 생성, 접속, 리스트 요청
struct lobbyRequest
{
	int msg_code;
	
	int uid;
	int rid;
};

//대기실 방 개수, write 두 번 쓰기는 아까워서 만듬
struct lobbyAuth
{
    int msg_code;

    int roomCount;
};

// 대기실 방 생성 승인
struct lobbyCreateAuth
{
	int msg_code;

	int rid;
};

//대기실 리스트 승인
struct lobbyListAuth
{
	int msg_code;

	struct room room; // [4] 에서 변경  

};
//대기실 리스트 승인  버퍼용
struct lobbyListAuth_2
{
	int msg_code;

	struct room room[4];

};

//대기실 방 접속 승인
struct lobbyEnterAuth
{
	int msg_code;

	int slot;
	int ucnt;
    struct user user[3];

};

//유저 브로드캐스트
struct enterRoomBroadcast
{
	int msg_code;

    struct user user;

};
#endif

#ifndef INROOM
#define INROOM

//방 클라이언트의 메시지 원형 12byte
struct inRoomMsg
{
	int msg_code;

	char data[8];
};

//방 요청 , 상태 변경, 게임 시작, 방 퇴장 요청
struct inRoomRequest
{
	int msg_code;
	
	int rid;
	int stats;
};

//게임 시작 승인
struct inRoomStartAuth
{
	int msg_code;

	int portNum; // udp 포트 번호
};

//유저가 상태를 변경했다고 다른 유저들에게 알림
struct inRoomStateBroadcast
{
    int msg_code;

    int slot;
    int stats;
};


#endif
#endif
