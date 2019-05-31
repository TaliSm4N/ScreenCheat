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


//각 구조체에 대한 LOG형태도 추가되어야함
