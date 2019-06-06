#ifndef MSG_H
#define MSG_H

//profile에 대한 구조체
struct profile
{
	char id[16];
	int win;
	int lose;
	int kill;
	int death;
};

//user에 대한 구조체, inRoom 에서 접속시 전송
struct user //[3]
{
	char id[16];
	int slot;
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

//로그인 메시지 원형 40byte
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

	int sid;
	struct profile profile;
};

#endif

#ifndef LOBBY
#define LOBBY


//대기실 메시지 원형 120byte
struct lobbyMsg
{
	int msg_code;

	char data[116];
};

//대기실 요청 , 방 생성, 접속, 리스트 요청
struct lobbyRequest
{
	int msg_code;
	int rid;
	int sid;
};

//대기실 방 개수, write 두 번 쓰기는 아까워서 만듬
struct lobbyAuth
{
    int msg_code;
    int roomCount;
};

//대기실 생성, 리스트 승인
struct lobbyListAuth//[4]
{
	int msg_code;
	struct room room[4];

};

//대기실 접속 승인
struct lobbyEnterAuth//[3]
{
	int msg_code;
    struct user user[3];

};


#endif

#endif
