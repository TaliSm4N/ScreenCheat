#ifndef UDPMSG_H
#define UDPMSG_H

typedef struct location
{
	float x;
	float y;
	float z;
}loc;

typedef struct udpMsg
{
	int id;
	loc pos;
	loc cam;
	int atk;
}udpMsg;

#endif
