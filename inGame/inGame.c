#include "inGame.h"


void *inGame_main(void *arg)
{
	int *clnt_sock=(*((socketData *)arg)).clnt_sock;
	int clntCnt=(*((socketData *)arg)).clntCnt;

	//int udp_sock;
	struct sockaddr_in udp_adr;

	struct epoll_event *ep_events;
	struct epoll_event event;
	int epfd,event_cnt;

	//test msg
	int str_len;
	char buf[100];


	epfd = epoll_create(EPOLL_SIZE);
	ep_events = malloc(sizeof(struct epoll_event)*EPOLL_SIZE);
	printf("make epoll %d\n",epfd);
	printf("clnt cnt : %d\n",clntCnt);

	//udp_sock=UDPSet(9191,&udp_adr);
	
	//epoll set
	for(int i=0;i<clntCnt;i++)
	{
		printf("fd : %d\n",clnt_sock[i]);
		event.events=EPOLLIN;
		event.data.fd=clnt_sock[i];
		epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_sock[i],&event);
	}

	//TCP mux
	while(1)
	{
		printf("epoll wait\n");
		event_cnt = epoll_wait(epfd,ep_events,EPOLL_SIZE,-1);

		if(event_cnt==-1)
		{
			puts("epoll_wait() error");
			break;
		}

		for(int i=0;i<event_cnt;i++)
		{
			str_len=read(ep_events[i].data.fd,buf,100);

			for(int j=0;j<clntCnt;j++)
			{
				if(ep_events[i].data.fd!=clnt_sock[j])
					write(clnt_sock[j],buf,str_len);
			}
			
		}
	}

	close(epfd);

	return arg;
}

int UDPSet(int port,struct sockaddr_in *udp_adr)
{
	int udp_sock;

	udp_sock=socket(PF_INET,SOCK_DGRAM,0);

	if(udp_sock==-1)
	{
		fprintf(stderr,"UDP socket creation error\n");
		return -1;
	}

	memset(udp_adr, 0, sizeof(*udp_adr));
	udp_adr->sin_family=AF_INET;
	udp_adr->sin_addr.s_addr = htonl(INADDR_ANY);
	udp_adr->sin_port=htons(port);

	if(bind(udp_sock, (struct sockaddr *)udp_adr, sizeof(*udp_adr))==-1)
	{
		fprintf(stderr,"bind error\n");
		close(udp_sock);
		return -1;
	}

	printf("success udp socket  %d\n",udp_sock);

	return udp_sock;
}

void *UDP_func(void *arg)
{
	int udp_sock=((udpDataSet *)arg)->sock;
	int clntCnt = ((udpDataSet *)arg)->clntCnt;
	struct sockaddr_in clnt_adr[4];

	for(int i=0;i<PLAYER_CNT;i++)
		clnt_adr[i]=((udpDataSet *)arg)->clnt_addr[i];

	struct sockaddr_in send_adr;
	socklen_t send_adr_sz;

	int str_len;
	position pos_msg;

	
	while(1)
	{
		printf("UDP while\n");
		send_adr_sz=sizeof(send_adr);

		//udp 정보받아오기

		str_len=recvfrom(udp_sock,(char *)&pos_msg,sizeof(position),0,(struct sockaddr *)&send_adr,&send_adr_sz);

		printf("pos : x%f y%f z%f camera : x%f y%f z%f\n",pos_msg.ch.x,pos_msg.ch.y,pos_msg.ch.z,pos_msg.camera.x,pos_msg.camera.y,pos_msg.camera.z);
		
		for(int i=0;i<PLAYER_CNT;i++)
			sendto(udp_sock,(char *)&pos_msg,sizeof(position),0,(struct sockaddr *)&(clnt_adr[i]),send_adr_sz);

		//udp 되돌려보내기
	}
}
