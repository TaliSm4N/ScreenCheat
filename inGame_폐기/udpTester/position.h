#ifndef _POSITION_H
#define _POSITION_H

typedef struct location
{
	float x;
	float y;
	float z;
}loc;

typedef struct position
{
	int id;
	loc ch;
	loc camera;
	int attack;
	
}position;

#endif
