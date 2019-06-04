#include "main.h"

int main(int argc, char *argv[])
{
	int serv_sock;
	int str_len;

	socklen_t clnt_adr_sz;

	struct sockaddr_in serv_adr,clnt_adr;
	struct sockaddr_in clnt_adr_ary[4];
	int cnt;

	position now;
	position pos[4];

	if(argc!=2)
	{
		fprintf(stderr,"Usage : %s <port>\n",argv[0]);
		exit(1);
	}

	serv_sock=socket(PF_INET,SOCK_DGRAM,0);

	if(serv_sock == -1)
	{
		fprintf(stderr,"UDP socket creation error\n");
		exit(1);
	}

	memset(&serv_adr, 0,sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
	{
		fprintf(stderr,"bind error\n");
		exit(1);
	}
/*
	for(int i=0;i<PLAYER_CNT;i++)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		str_len = recvfrom(serv_sock,(char *)&now,sizeof(now),0,
				(struct sockaddr*)&clnt_adr,&clnt_adr_sz);
	}
*/

	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr);
		str_len = recvfrom(serv_sock,(char *)&now,sizeof(now),0,
				(struct sockaddr*)&clnt_adr,&clnt_adr_sz);

		logPosition(now);	
		now.id=2;
		now.ch.x+=100;
		now.camera.x*=-1;

		sendto(serv_sock,(char *)&now,sizeof(now),0,
				(struct sockaddr*)&clnt_adr,clnt_adr_sz);
	}
}
