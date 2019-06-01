#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "msg.h"

int main(int argc,char *argv[])
{
	int str_len;

	int sock;
	struct sockaddr_in serv_adr;

	struct loginRequest loginMsg;

	struct loginAuth auth;



	if(argc!=3)
	{
		fprintf(stderr,"Usage : %s <IP> <Port>\n",argv[0]);
		exit(1);
	}

	sock=socket(PF_INET,SOCK_STREAM,0);
	if(sock==-1)
	{
		fprintf(stderr,"socket error\n");
		exit(1);
	}

	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

	if(connect(sock,(struct sockaddr *)&serv_adr,sizeof(serv_adr))==-1)
	{
		fprintf(stderr,"connect erorr\n");
		exit(1);
	}
	fputs("join: 101, login: 100 \n", stdout);
	scanf("%d", &loginMsg.msg_code);
	fputs("id: ",stdout);
	scanf("%s",loginMsg.id);
	fputs("passward: ",stdout);
	scanf("%s",loginMsg.pwd);

	write(sock,&loginMsg,sizeof(struct loginRequest));

	printf("send msg \n");
	
	while(str_len==0)
	str_len=read(sock,&auth,sizeof(struct loginAuth));
	
	printf("auth.sid = %d\n", auth.sid);
	printf("str_len : %d\n",str_len);

	if(auth.sid==-1)
	{
		printf("login failed\n");
	}
	else
	{
		printf("login success\n");
		printf("sid : %d\n", auth.sid);
		printf("ID : %s\n",auth.profile.id);
		printf("win/lose : %d/%d\n",auth.profile.win,auth.profile.lose);
		printf("kill/death : %d/%d\n",auth.profile.kill,auth.profile.death);
	}
	
	
	while(1);
	close(sock);

	return 0;
}
