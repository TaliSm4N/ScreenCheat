#include "log.h"

//error 출력
void error_handling(char *buf)
{
	fputs(buf,stderr);
	fputc('\n',stderr);
	exit(1);
}

//buf 출력
void Log(char *buf)
{
	fputs(buf,stdout);
	fputc('\n',stdout);
}

//buf에 대한 number 출력
void LogNum(char *buf, int num)
{
	fprintf(stdout,"%s: %d\n",buf,num);
}

void LogUDPMsg(udpMsg msg)
{
	fprintf(stdout,"id : %d pos : (%f, %f, %f), cam : (%f, %f, %f)",msg.id,
			msg.pos.x,msg.pos.y,msg.pos.z,
			msg.cam.x,msg.cam.y,msg.cam.z);
	if(msg.atk==TRUE)
	{
		fprintf(stdout,"atk : TRUE\n");
	}
	else
	{
		fprintf(stdout,"atk : FALSE\n");
	}

}


//각 구조체에 대한 LOG형태도 추가되어야함
