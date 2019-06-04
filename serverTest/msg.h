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

#ifndef LOGIN
#define LOGIN

//로그인 메시지 원형
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

// 방 
struct room
{
	int rid;

	int ucount;
	char hname[16];
};

#endif


#endif
