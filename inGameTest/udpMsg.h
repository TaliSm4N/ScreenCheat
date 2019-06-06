#ifndef UDPMSG_H
#define UDPMSG_H

#include "location.h"

typedef struct udpMsg
{
	int id;
	loc pos;
	loc cam;
	int atk;
}udpMsg;

#endif
