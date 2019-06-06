#include "inGame.h"

void testIngame(int TCPport, int UDPport,int pCnt)
{
	int udp_sock;
	int epfd;
	int player[pCnt];//방장은 0번으로 함

	struct sockaddr_in clnt_adr[pCnt];
	//tcp연계부분 
	testTCP(player,pCnt,TCPport);

	initGame(0,player,pCnt);

	epfd=inGameEpoll(player,pCnt);
	


	udp_sock=setUDP(UDPport);

	//tcp를 통해 udp의 소켓 번호 전달(테스트용 생략)

	connectCheckUDP(udp_sock,clnt_adr,pCnt);
	playGame(udp_sock,clnt_adr,pCnt);

}

//실제 버전에서는 가지고 있는 tcp에 대해서 epoll로 만들지만
//테스트버전에서는 연결을 먼저해줘야함
int testTCP(int *sock_ary,int cnt,int port)
{
	int sock;
	struct sockaddr_in serv_adr, clnt_adr;
	socklen_t adr_sz;

	sock=socket(PF_INET,SOCK_STREAM,0);
	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(port);

	if(bind(sock,(struct sockaddr*)&serv_adr, sizeof(serv_adr))==-1)
		error_handling("tcp bind error");
	if(listen(sock,5)==-1)
		error_handling("tcp listen error");

	for(int i=0;i<cnt;i++)
	{
		adr_sz =sizeof(clnt_adr);
		sock_ary[i]=accept(sock,(struct sockaddr*)&clnt_adr,&adr_sz);
		LogNum("connect TCP",i);

	}
	return sock;
}

int inGameEpoll(int *player,int cnt)
{
	int epfd;
	struct epoll_event event;

	epfd=epoll_create(EPOLL_SIZE);
	
	for(int i=0;i<cnt;i++)
	{	
		event.events=EPOLLIN;
		event.data.fd=player[i];
		epoll_ctl(epfd,EPOLL_CTL_ADD,player[i],&event);
	}

	return epfd;
}

int initGame(int hostNum, int *player,int cnt)
{
	inGameMap mapMsg;
	inGameUser userMsg;
	//loc obj[OBJ_CNT];

	Log("initGame");

	do
	{
		read(player[hostNum],(char *)&mapMsg,sizeof(inGameMap));
		LogNum("ID",mapMsg.obj_num);
		LogLoc(mapMsg.location);

		for(int i=0;i<cnt;i++)
		{
			if(i!=hostNum)
			{
				mapMsg.msg_code=410;
				write(player[i],(char *)&mapMsg,sizeof(inGameMap));
			}
		}
	}while(mapMsg.remainder>1);

	Log("get all map msg");

	do
	{
		read(player[hostNum],(char *)&userMsg,sizeof(inGameUser));
		LogLoc(userMsg.location);

		for(int i=0;i<cnt;i++)
		{
			if(i!=hostNum)
			{
				userMsg.msg_code=411;
				write(player[i],(char *)&userMsg,sizeof(inGameUser));
			}
		}
	}while(userMsg.remainder>1);
	Log("get all user msg");

}

void *tcp_thread(void *arg)
{
	int epfd;
//	int player[]

	return NULL;
}

int setUDP(int port)
{
	int serv_sock;

	struct sockaddr_in serv_adr;
	
	serv_sock = socket(PF_INET,SOCK_DGRAM,0);

	if(serv_sock == -1)
	{
		error_handling("UDP socket make failed");
	}

	memset(&serv_adr, 0,sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port=htons(port);

	if(bind(serv_sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
	{
		error_handling("UDP socket bind failed");
	}
	
	return serv_sock;
}

int connectCheckUDP(int sock,struct sockaddr_in *clnt_adr,int cnt)
{
	udpMsg msg;

	socklen_t clnt_adr_sz;

	srand(time(NULL));

	for(int i=0;i<cnt;i++)
	{
		clnt_adr_sz=sizeof(clnt_adr[i]);
		recvfrom(sock,(char *)&msg,sizeof(udpMsg),0,
				(struct sockaddr*)&(clnt_adr[i]),&clnt_adr_sz);

		msg.id = i;
		msg.pos.x=(float)(rand()%51-25);
		msg.pos.z=(float)(rand()%51-25);
		msg.pos.y=1.0;

		LogUDPMsg(msg);

		sendto(sock,(char*)&msg,sizeof(udpMsg),0,(struct sockaddr *)&(clnt_adr[i]),clnt_adr_sz);
	}

	return TRUE;
}


int playGame(int sock,struct sockaddr_in *clnt_adr,int cnt)
{

	udpMsg msg;

	struct sockaddr_in recv_adr;
	socklen_t clnt_adr_sz =sizeof(recv_adr);

	int i;
	while(1)
	{
		clnt_adr_sz = sizeof(recv_adr);

		recvfrom(sock,(char *)&msg,sizeof(udpMsg),0,
(struct sockaddr *)&recv_adr,&clnt_adr_sz);

		LogUDPMsg(msg);

		for(i=0;i<cnt;i++)
		{
			if(i!=msg.id)
			{
				sendto(sock,(char *)&msg,sizeof(udpMsg),0,
						(struct sockaddr*)&(clnt_adr[i]),clnt_adr_sz);
			}
		}
	}

	return TRUE;
}
