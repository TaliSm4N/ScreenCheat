#include "main.h"

int main(int argc,char *argv[])
{
	if(argc!=3)
	{
		error_handling("need tcp udp port num");
	}

	testIngame(atoi(argv[1]),atoi(argv[2]),PLAYER_CNT);

	Log("main End");

	return 0;
}
