#include "inGame.h"
#include <pthread.h>


void testIngame(int TCPport, int UDPport,int pCnt)
{
	int udp_sock;
	int epfd;
	int player[pCnt];//방장은 0번으로 함

	int play=TRUE;

	struct tcpThreadArg arg;

	pthread_t t_id;

	struct sockaddr_in clnt_adr[pCnt];
	//tcp연계부분 
	testTCP(player,pCnt,TCPport);

	initGame(0,player,pCnt);
	
	Log("make epoll");
	epfd=inGameEpoll(player,pCnt);
	Log("make epoll success");

	udp_sock=setUDP(UDPport);

	//tcp를 통해 udp의 소켓 번호 전달(테스트용 생략)

	arg.player=player;
	arg.pCnt=pCnt;
	arg.play = &play;

	if(pthread_create(&t_id,NULL,tcp_thread,&arg)!=0)
	{
		error_handling("tcp inGame thread error");
	}

	connectCheckUDP(udp_sock,player,clnt_adr,pCnt);
	playGame(udp_sock,clnt_adr,pCnt,&play);
	

	//test함수에서는 tcp를 모두 종료
//	for(int i=0;i<pCnt;i++)
//	{
//		close(player[i]);
//	}
	close(epfd);
	Log("testMode End");
	while(1);
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
		LogNum("sock_ary[i]",sock_ary[i]);

	}
	return sock;
}

int inGameEpoll(int *player,int cnt)
{
	int epfd;
	struct epoll_event event;

	int flag;

	epfd=epoll_create(EPOLL_SIZE);
	
	for(int i=0;i<cnt;i++)
	{	
		flag=fcntl(player[i],F_GETFL,0);
		fcntl(player[i],F_SETFL,flag|O_NONBLOCK);
		event.events=EPOLLIN|EPOLLET;
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

	int size;

	Log("initGame");

	Log("Object position set");

	do
	{
		size = read(player[hostNum],(char *)&mapMsg,sizeof(inGameMap));


		LogNum("ID",mapMsg.obj_num);
		LogLoc(mapMsg.location);
		LogNum("remain",mapMsg.remainder);

		for(int i=0;i<cnt;i++)
		{
	//		if(i!=hostNum)
	//		{
				mapMsg.msg_code=MAP_SERV;
				write(player[i],(char *)&mapMsg,sizeof(inGameMap));
	//		}
		}
	}while(mapMsg.remainder>=1);

	Log("get all map msg");

	Log("User position set");

	do
	{
		read(player[hostNum],(char *)&userMsg,sizeof(inGameUser));
		LogLoc(userMsg.location);
		LogNum("remain",userMsg.remainder);

		for(int i=0;i<cnt;i++)
		{
//			if(i!=hostNum)
//			{
				userMsg.msg_code=USR_SERV;
				write(player[i],(char *)&userMsg,sizeof(inGameUser));
//			}
		}
	}while(userMsg.remainder>=1);
	Log("get all user msg");

}

void *tcp_thread(void *arg)
{
//	int epfd=((struct tcpThreadArg *)arg)->epfd;
	int epfd;
	int *player=((struct tcpThreadArg *)arg)->player;
	int pCnt=((struct tcpThreadArg *)arg)->pCnt;
	int *play = ((struct tcpThreadArg *)arg)->play;
	struct gameInfo info[4];

	inGameMsg msg;
//	inGameAtk atkMsg;
	inGameItemSend itemMsg;

	struct epoll_event event;
	struct epoll_event *ep_events;
	int event_cnt,i,j;

	int str_len;

	//info 초기화
	for(int i=0;i<pCnt;i++)
	{
		info[i].kill=0;
		info[i].death=0;
		info[i].win=0;
	}

	epfd=inGameEpoll(player,pCnt);
	ep_events=malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

	Log("tcp Thread start");

	while(1)
	{
		event_cnt=epoll_wait(epfd,ep_events,EPOLL_SIZE,-1);
		if(event_cnt==-1)
		{
			Log("epoll_wait error");
			break;
		}

		for(i=0;i<event_cnt;i++)
		{
//			while(1)
//			{
				str_len=read(ep_events[i].data.fd,(char *)&msg,sizeof(inGameMsg));
				
				//제대로 동작 안하면 삭제
//				if(str_len<0)
//				{
//					if(errno==EAGAIN)
//						break;
//				}
//			}
			Log("--------------------");
			LogNum("sender",ep_events[i].data.fd);
			LogNum("msg_code",msg.msg_code);
			Log("--------------------");
			switch(msg.msg_code)
			{
				case ATK_CLNT:
					//attack의 경우 보내는 것과 받는 것이 같음
					msg.msg_code=ATK_SERV;
					tcpMsgSend(player,pCnt,&msg,-1);

					if(((inGameAtk *)&msg)->dmg==-1)
					{
						Log("death");
						info[((inGameAtk *)&msg)->tid].death++;
						LogNum("dead man's death",info[((inGameAtk *)&msg)->tid].death);
						info[((inGameAtk *)&msg)->sid].kill++;
						LogNum("killer's kill",info[((inGameAtk *)&msg)->sid].kill);

					}
					LogNum("attacker",((inGameAtk *)&msg)->sid);
					LogNum("target",((inGameAtk *)&msg)->tid);
					LogNum("damage",((inGameAtk *)&msg)->dmg);
					break;
				case ITEM_CLNT:
					itemMsg.msg_code=ITEM_SERV;
					itemMsg.icode = ((inGameItemRecv *)&msg)->icode;
					tcpMsgSend(player,pCnt,(inGameMsg *)&itemMsg,-1);
					break;
				case RESPAWN_CLNT:
					msg.msg_code=RESPAWN_SERV;
					
					tcpMsgSend(player,pCnt,&msg,-1);

					LogNum("respawn",ep_events[i].data.fd);
					break;
				case END_CLNT:
					msg.msg_code = END_SERV;

					int maxKill=-1;
					int winner=-1;

					for(int j=0;j<pCnt;j++)
					{
						if(info[j].kill>maxKill)
						{
							winner=j;
							maxKill=info[j].kill;
						}
						epoll_ctl(epfd,EPOLL_CTL_DEL,player[j],NULL);
					}

					Log("end game");

					((inGameEnd *)&msg)->winner=winner;
					info[winner].win=1;

					tcpMsgSend(player,pCnt,&msg,-1);
					resultWrite(info,pCnt);

					close(epfd);

					*play=FALSE;
					return NULL;
			}
		}
	}

	return NULL;
}

//send가 기존 fd값이 아니면 모두에게 보냄
//통일성 있게 이 값은 0으로 사용
void tcpMsgSend(int *fd,int pCnt,inGameMsg *msg,int send)
{
	int str_len;
	LogNum("non send fd",send);
	for(int i=0;i<pCnt;i++)
	{
		if(fd[i]!=send)
		{
			str_len=write(fd[i],(char *)msg,sizeof(inGameMsg));
			LogNum("send message",i);
			LogNum("send fd",fd[i]);
			LogNum("str_len",str_len);
		}
	}
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

	Log("UDP setting");
	
	return serv_sock;
}

int connectCheckUDP(int sock,int *player,struct sockaddr_in *clnt_adr,int cnt)
{
	int str_len;
	//inGameMsg checkMsg;
	inGameUDPCheck checkMsg;
	udpMsg msg;

	socklen_t clnt_adr_sz;

	checkMsg.msg_code=UDPCHECK;

	srand(time(NULL));

	Log("wait UDP msg");
	LogNum("cnt",cnt);

	for(int i=0;i<cnt;i++)
	{
		checkMsg.id=-1;
		str_len=write(player[i],(char *)&checkMsg,sizeof(inGameUDPCheck));
	//	LogNum("msg_code",checkMsg.msg_code);
		Log("--------------");
		LogNum("player",player[i]);
		LogNum("id",checkMsg.id);
		LogNum("waiting",str_len);
		clnt_adr_sz=sizeof(clnt_adr[i]);
		while(1)
		{
			str_len=recvfrom(sock,(char *)&msg,sizeof(udpMsg),0,
				(struct sockaddr*)&(clnt_adr[i]),&clnt_adr_sz);
			if(str_len!=-1)
				break;
			Log("get udp check failed");
		}
		LogNum("sin_family",clnt_adr[i].sin_family);
		LogNum("port",clnt_adr[i].sin_port);
		LogNum("addr",clnt_adr[i].sin_addr.s_addr);
		LogNum("get UDP msg",i);
		Log("--------------");
		LogUDPMsg(msg);
		checkMsg.id=i;
		write(player[i],(char *)&checkMsg,sizeof(inGameUDPCheck));
		sendto(sock,(char*)&msg,sizeof(udpMsg),0,(struct sockaddr *)&(clnt_adr[i]),clnt_adr_sz);
	}

	Log("Get all UDP");

	return TRUE;
}


int playGame(int sock,struct sockaddr_in *clnt_adr,int cnt,int *play)
{

	udpMsg msg;

	struct sockaddr_in recv_adr;
	socklen_t clnt_adr_sz =sizeof(recv_adr);

	int str_len;

	int i;

	int sendCnt=0;

	Log("udp thread start");
	LogNum("playerCnt",cnt);
	while(1)
	{
		if(*play==FALSE)
		{
			Log("gameEnd");
			close(sock);//udp socket 제거
			break;
		}
		clnt_adr_sz = sizeof(recv_adr);

		recvfrom(sock,(char *)&msg,sizeof(udpMsg),0,
(struct sockaddr *)&recv_adr,&clnt_adr_sz);

	//	LogUDPMsg(msg);

		sendCnt=0;
		for(i=0;i<cnt;i++)
		{
			if(i!=msg.id)
			{
				str_len=sendto(sock,(char *)&msg,sizeof(udpMsg),0,
						(struct sockaddr*)&(clnt_adr[i]),clnt_adr_sz);
				sendCnt++;
		//		LogNum("i",i);
		//		LogNum("str_len",str_len);
			}
	//			Log("====================");
	//			LogNum("sender",msg.id);
	//			LogNum("reciever",i);
	//			Log("====================");
		}
	//	Log("------------------");
	//	LogNum("sendCnt",sendCnt);
	//	LogNum("msg id",msg.id);
	//	LogUDPMsg(msg);
	//	Log("----------------");
	}

	return TRUE;
}

void resultWrite(struct gameInfo *info,int pCnt)
{
	//DB와 연결 후 게임 결과를 기록

	//현재는 아무 역할도 안함

	return;
}
