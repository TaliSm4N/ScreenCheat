#include "main.h"

int main(int argc,char *argv[])
{
	if(argc!=2)
	{
		error_handling("need port num");
	}

	testIngame(0,atoi(argv[1]),PLAYER_CNT);

	return 0;
}
