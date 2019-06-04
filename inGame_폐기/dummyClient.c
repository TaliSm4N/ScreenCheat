#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include "position.h"

#define BUF_SIZE 1024

void error_handling(char *message);
void *listening(void *arg);
void *UDPClient(void *arg);

int sock;
int udp_sock;

struct sockaddr_in UDP_adr,from_adr;

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_adr;

	pthread_t t_id;
	pthread_t udp_t_id;
	
	//test set
	char message[BUF_SIZE];
	int str_len;

	char udpPort[5];

	if(argc!=3)
	{
		printf("Usage : %s <IP> <port>\n",argv[0]);
		exit(1);
	}

	sock=socket(PF_INET,SOCK_STREAM,0);
	if(sock==-1)
		error_handling("socket() error");

	memset(&serv_adr, 0,sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=inet_addr(argv[1]);
	serv_adr.sin_port=htons(atoi(argv[2]));

	if(connect(sock, (struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("connect() error!");
	else
		puts("Connected.........");

	
	
	read(sock,udpPort,5);
	printf("udpPortNum : %s\n",udpPort);
	
	udp_sock=socket(PF_INET,SOCK_DGRAM,0);

	memset(&UDP_adr,0,sizeof(UDP_adr));
	UDP_adr.sin_family=AF_INET;
	UDP_adr.sin_addr.s_addr=inet_addr(argv[1]);
	UDP_adr.sin_port=htons(atoi(udpPort));

	sendto(udp_sock,"test",strlen("test"),0,(struct sockaddr *)&UDP_adr,sizeof(UDP_adr));
	

	if(pthread_create(&udp_t_id, NULL,UDPClient,NULL)!=0)
	{
		error_handling("UDP thread error\n");
	}

	if(pthread_create(&t_id, NULL,listening,NULL)!=0)
	{
		error_handling("thread error\n");
	}

	//tcp

	while(1)
	{
	//	fgets(message,BUF_SIZE,stdin);
	//	write(sock,message,strlen(message));
	}

	close(sock);
}

void error_handling(char *message)
{
	fputs(message, stderr);
	fputc('\n', stderr);
	exit(1);
}

void *listening(void *arg)
{
	int str_len;
	char message[BUF_SIZE];
	position now;

	socklen_t adr_sz;

	while(1)
	{
		//udp
		adr_sz=sizeof(from_adr);
		str_len=recvfrom(udp_sock,(char *)&now,sizeof(now),0,(struct sockaddr *)&from_adr,&adr_sz);

		printf("pos : x%f y%f z%f camera : x%f y%f z%f\n",now.ch.x,now.ch.y,now.ch.z,now.camera.x,now.camera.y,now.camera.z);

		//tcp
//		str_len=read(sock,message,BUF_SIZE-1);
//		message[str_len]=0;
//		printf("message from server : %s",message);
	}
}

void *UDPClient(void *arg)
{
	position now;
	char msg[10];

//	now.ch={0.0,0.0,0.0};
//	now.camera={0.0,0.0,0.0};
	
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

		sendto(udp_sock,(char *)&now,sizeof(now),0,(struct sockaddr *)&UDP_adr,sizeof(UDP_adr));

	}

}
