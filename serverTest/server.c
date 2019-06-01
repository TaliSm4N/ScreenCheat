#include "server.h"

void Signal_Init(void) // 시그널 처리 함수, 종료되는 시그널이 발생한 경우 클라이언트의 소켓을 회수하고, epoll을 해제한다
{
	struct sigaction act_ignore1;
	struct sigaction act_ignore2;
	memset(&act_ignore1, 0x00, sizeof(struct sigaction));
	memset(&act_ignore2, 0x00, sizeof(struct sigaction));
	
	act_ignore1.sa_handler = closeClient(epfd, eFD);
	sigemptyset(&(act_ignore1.sa_mask));
	act_ignore1.sa_flags = 0;
	
	act_ignore2.sa_handler = closeClient(epfd, eFD);
	sigemptyset(&(act_ignore2.sa_mask));
	act_ignore2.sa_flags = 0;
	
	sigaction(SIGPIPE, &act_ignore1, NULL);
	sigaction(SIGINT, &act_ignore2, NULL); //Ctrl-C
	
	
	
}

//SIGPIPE 처리
/*void sigpipeIgnore(int signo)
{
	char buf[200];
	memset(buf, 0x00, sizeof(buf));
    snprintf(buf, 200, "Signal Ignore : SIGPIPE(%d)", signo);
	Log(buf);
} */

//SIGINT 처리, 클라이언트 소켓을 닫고 epoll을 해제한다
/*void sigintIgnore()
{
	Log("sigint Ignored");
}*/

void setnonblockingmode(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag|O_NONBLOCK);
}

//listening socket setting
int setListening(char *port)
{
	int sock;
	struct sockaddr_in serv_adr;
	
	Log("setListening socket");
	LogNum("port",atoi(port));

	if(port==NULL)
		return -1;

	sock=socket(PF_INET,SOCK_STREAM,0);

	if(sock==-1)
	{
		error_handling("listening socket error");
	}

	memset(&serv_adr,0,sizeof(serv_adr));
	serv_adr.sin_family=AF_INET;
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(port));

	if(bind(sock,(struct sockaddr*)&serv_adr,sizeof(serv_adr))==-1)
	{
		error_handling("bind error");
	}

	if(listen(sock, 5)==-1)
	{
		error_handling("listen error");
	}

	Log("setListening socket success");
	LogNum("socket fd",sock);

	return sock;
}

//epoll setting
int setEpoll(int sock)
{
	int epfd=epoll_create(EPOLL_SIZE);

	struct epoll_event event;
	struct eventData *eData;
	eData=(struct eventData *)malloc(sizeof(struct eventData)*1);

	Log("epoll setting");

	//socket setting
	setnonblockingmode(sock); // 비동기화
	event.events = EPOLLIN;
	eData->fd=sock; 
	eData->state=0;
	event.data.ptr = (void *)eData;

	epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &event); // 관심대상 추가

	Log("epoll setting success");
	LogNum("epoll fd",epfd);


	return epfd;
}

//client 소켓 생성
int connectClient(int sock,int epfd)
{
	int clnt_sock;
	struct sockaddr_in clnt_adr;
	socklen_t adr_sz;

	struct epoll_event event;
	struct eventData *eData;
	eData=malloc(sizeof(struct eventData));

	adr_sz=sizeof(clnt_adr);
	clnt_sock=accept(sock,(struct sockaddr*)&clnt_adr,&adr_sz);
	setnonblockingmode(clnt_sock);
	event.events=EPOLLIN|EPOLLET;
	eData->fd=clnt_sock;
	eData->state=0; //어떤 모듈에 있는지 0 Login 모듈 1대기실 모듈 2 방모듈 3 인게임 4 종료 
	event.data.ptr=(void *)eData;

	epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_sock,&event);

	Log("client connect");
	LogNum("client fd",clnt_sock);

	return clnt_sock;
}
//클라이언트 소켓을 닫고 epoll을 해제한다, SIGINT 발생 시에도 동일하게 처리
int closeClient(int epfd,int fd)
{
	LogNum("Closed Client Socket", fd);
	epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL); // epoll 관리 대상에서 제외
	close(fd);
}

//login module msg처리
int login(int fd)
{
	int str_len;
	struct loginMsg loginMsg;
	struct loginRequest *request;
	struct loginAuth auth;

	Log("Login module");
	LogNum("fd",fd);

	str_len=read(fd,&loginMsg,sizeof(struct loginMsg));




	switch(loginMsg.msg_code)
	{
		case 100:// 로그인 요청
			//id, password 비교
			Log("request Login");
			request=(struct loginRequest *)&loginMsg;

			Log("id");

			auth.msg_code=110;

			if (compareID (request->id, request->pwd, &auth.profile, &auth.sid))
			{
				Log("success Login");
				write(fd,&auth,sizeof(struct loginAuth)); // 로그인에 성공한 클라이언트 정보 전송
			}
			
			else {
				Log("failed Login");
				write(fd,&auth,sizeof(struct loginAuth)); // 실패 했으므로 sid -1 전송
			}
			
			break;

		case 101: // 회원가입 요청
			Log("request joinMembership");
			request=(struct loginRequest *)&loginMsg;
			auth.msg_code=111;
			
			joinMembership(request->id, request->pwd);
			write(fd,&auth,sizeof(struct loginAuth)); // 회원가입 확인용 블록을 막기위해서 전송한다.
			break;
			
		case 501: // 게임 종료 요청, eState를 4로 바꿔야함
			Log("request Quit Game");
			break;
	}


	return 0;
}

//server main code
int server(char *port)
{
	int i;

	int sock;
	int epfd;

	int event_cnt;
	struct epoll_event	*ep_events=(struct epoll_event *)malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

	Signal_Init();
	
	sock=setListening(port);//3
	epfd=setEpoll(sock);//4
	connectDB();
	Log("server");

	while(1)
	{
		event_cnt=epoll_wait(epfd,ep_events,EPOLL_SIZE,-1);
		if(event_cnt == -1)
		{
			Log("epoll_wait error");
			break;
		}

		printf("event_cnt : %d\n",event_cnt);

		for(i=0;i<event_cnt;i++)
		{
			if(eFD==sock)
			{
				connectClient(sock,epfd); //클라이언트 소켓 생성
			}
			else if(eState == 0)//로그인 모듈일 때
			{
				login(eFD);
				
			}
			
			else if(eState == 4) // 종료 요청
			{
				closeClient(epfd, eFD);
			}
		}
	//	break;
	}
	
	closeDB();
	close(epfd);

	return 0;
}
