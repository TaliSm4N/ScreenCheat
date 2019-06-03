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
	
	while(1) {
		fputs("join: 101, login: 100, quit: 500 \n", stdout);
		scanf("%d", &loginMsg.msg_code);
		fputs("id: ",stdout);
		scanf("%s",loginMsg.id);
		fputs("passward: ",stdout);
		scanf("%s",loginMsg.pwd);

		write(sock,&loginMsg,sizeof(struct loginRequest));

		printf("send msg \n");
		while(str_len==0)
			str_len=read(sock,&auth,sizeof(struct loginAuth));
		//str_len=read(sock,&auth.sid,sizeof(int));
		printf("auth.sid = %d\n", auth.sid);
		printf("str_len : %d\n",str_len);
		printf("auth.msg_code : %d\n",auth.msg_code);
		
		switch(auth.msg_code) // 서버에서 온 메시지 코드에 따른 처리
		{	
			case 110: // 로그인 성공
				if (auth.msg_code == 110 && auth.sid == -1)
				{
				printf("login failed\n");
				break;
				}
				printf("login success\n");
				printf("sid : %d\n", auth.sid);
				printf("ID : %s\n",auth.profile.id);
				printf("win/lose : %d/%d\n",auth.profile.win,auth.profile.lose);
				printf("kill/death : %d/%d\n",auth.profile.kill,auth.profile.death);
			break;
			
			case 111: // 회원가입 성공
				printf("join successs\n");
			break;
			
			case 112: // 대기실 진입 요청
				printf("fuck");
			break;
			
			case 113: // 회원가입 실패
				printf("join failed\n");
			break;
			
			case 510: // 종료 요청
				close(sock);
				return 0;
		
		}
	//	memset(&auth,0,sizeof(struct loginAuth));	
	}
}
