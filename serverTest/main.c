#include "main.h"

int main(int argc, char *argv[])
{
	Log("start server");

	if(argc!=2)
	{
		error_handling("need <port> num");
	}
	
	//server 실행
	server(argv[1]);

	return 0;
}
