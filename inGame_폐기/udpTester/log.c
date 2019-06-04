#include "log.h"

void logPosition(position pos)
{
	printf("pos : x%f y%f z%f camera : x%f y%f z%f\n",pos.ch.x,pos.ch.y,pos.ch.z,pos.camera.x,pos.camera.y,pos.camera.z);

	return;
}
