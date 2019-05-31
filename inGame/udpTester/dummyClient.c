#include "main.h"

int main(int argc, char *argv[])
{
	int sock;

	int str_len;
	socklen_t adr_sz;

	char msg[10];

	position now;

	now.camera.z=0.0f;

	struct sockaddr_in serv_adr, from_adr;

	if(argc!=3)
	{
		fprintf(stderr,"Usage : %s <IP> <port>\n",argv[0]);
		exit(1);
	}

	sock = socket(PF_INET,SOCK_DGRAM,0);

	if(sock==-1)
	{
		fprintf(stderr,"socket error\n");
		exit(1);
	}

	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

	while(1)
	{
		scanf("%s",msg);

		switch(msg[0])
		{
			case 'w':
				now.ch.x+=0.5;
				break;
			case 's':
				now.ch.x-=0.5;
				break;
			case 'a':
				now.ch.y-=0.5;
				break;
			case 'd':
				now.ch.y+=0.5;
				break;
			case 'q':
				now.ch.z+=0.5;
				break;
			case 'e':
				now.ch.z-=0.5;
				break;
		}
		sendto(sock,(char *)&now,sizeof(now),0,(struct sockaddr*)&serv_adr,sizeof(serv_adr));

		logPosition(now);
	}
}
