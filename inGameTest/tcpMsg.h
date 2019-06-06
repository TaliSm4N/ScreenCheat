#ifndef TCPMSG_H
#define TCPMSG_H

#include "location.h"

#ifndef INGAME
#define INGAME

typedef struct inGameMsg
{
	int msg_code;

	char data[20];
}inGameMsg;

typedef struct inGameMap
{
	int msg_code;
	int obj_num;
	loc location;
	int remainder;
}inGameMap;

typedef struct inGameUser
{
	int msg_code;
	int uid;
	loc location;
	int remainder;
}inGameUser;

typedef struct inGameAtk
{
	int msg_code;
	int sid;
	int tid;
	int dmg;
	char padding[8];
}inGameAtk;

typedef struct inGameItemRecv
{
	int msg_code;
	int sid;
	int icode;
	char padding[12];
}inGameItemRecv;

typedef struct inGameItemSend
{
	int msg_code;
	int icode;
	char padding[16];
}inGameItemSend;

#endif


#endif
