#include "server.h"

//SIGINT 처리
void sigInt_handler (void)
{
	struct sigaction act_ignore;
	memset(&act_ignore, 0x00, sizeof(struct sigaction));


	act_ignore.sa_handler = sigintIgnore;
	sigemptyset(&(act_ignore.sa_mask));
	act_ignore.sa_flags = 0;

	sigaction(SIGINT, &act_ignore, NULL); //Ctrl-C

}

/* 안되서 주석처리 나중에 다시 생각해보기로 한다.
//SIGPIPE 처리, 종료되는 시그널이 발생한 경우 클라이언트의 소켓을 회수하고, epoll을 해제한다
void sigPipe_handler(int epfd, int fd)
{
	struct sigaction act_ignore2;
	memset(&act_ignore2, 0x00, sizeof(struct sigaction));

	//Log("SIGPIPE has occured!");
	act_ignore2.sa_handler = closeClient(epfd, fd);
//SIG_IGN;
	sigemptyset(&(act_ignore2.sa_mask));
	act_ignore2.sa_flags = 0;

	sigaction(SIGPIPE, &act_ignore2, NULL);

} */

void sigintIgnore(int signo)
{
	Log("sigintIgnored");
}


// 비동기 소켓으로 설정
void setnonblockingmode(int fd)
{
	int flag = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flag|O_NONBLOCK);
}

//listening socket setting
int setListening(char *port)
{
	int sock, option;
	struct sockaddr_in serv_adr;
    socklen_t optlen;
	Log("setListening socket");
	LogNum("port",atoi(port));

	if(port == NULL)
		return -1;

	sock=socket(PF_INET,SOCK_STREAM, 0);

	if(sock == -1)
	{
		error_handling("listening socket error");
	}
    // bind error 처리, 안되던데 왜 안되는지는 모름
    optlen = sizeof(option);
	option = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, optlen);

	memset(&serv_adr, 0, sizeof(serv_adr));
	serv_adr.sin_family = AF_INET;
	serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_adr.sin_port = htons(atoi(port));

	if(bind(sock,(struct sockaddr*)&serv_adr, sizeof(serv_adr)) == -1)
	{
		error_handling("bind error");
	}

	if(listen(sock, 5) == -1)
	{
		error_handling("listen error");
	}

	Log("setListening socket success");
	LogNum("socket fd", sock);

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
void closeClient(int epfd,int fd)
{
	LogNum("Closed Client Socket", fd);
	epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL); // epoll 관리 대상에서 제외
	close(fd);
	//return 0;
}

//login module msg처리
int login(int epfd ,int fd)
{
	int str_len;
	struct loginMsg loginMsg;
	struct loginRequest *request;
	struct loginAuth auth;

	Log("Login module");
	LogNum("fd",fd);
	//sigPipe_handler(epfd,fd);

	str_len=read(fd,&loginMsg,sizeof(struct loginMsg)); // 로그인 단계 메시지를 받음

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
			//	write(fd,&auth,sizeof(int*2)); // 실패 했으므로 sid -1 전송
				write(fd,&auth,sizeof(struct loginAuth)); // 실패 했으므로 sid -1 전송
			}

			break;

		case 101: // 회원가입 요청
			Log("request joinMembership");
			request=(struct loginRequest *)&loginMsg;

			if (joinMembership(request->id, request->pwd) == 1) // 회원가입에 성공함
			{
				auth.msg_code=111;
			}

			else { //회원가입에 실패함
				printf("join failed\n");
				auth.msg_code=113;
			}
			write(fd,&auth.msg_code,sizeof(int)); // 회원가입 확인용 블록을 막기위해서 전송한다.
			//write(fd,&auth,sizeof(struct loginAuth)); // 회원가입 확인용 블록을 막기위해서 전송한다.
			break;

		case 102: // 대기실 상태로 전환한다.
			Log("request change to Lobby");
			auth.msg_code=112;
			write(fd,&auth.msg_code,sizeof(int));
			//write(fd,&auth,sizeof(struct loginAuth));
			return 1;
			break;

		case 500: // 게임 종료 요청, eState를 4로 바꿔야함
			Log("request Quit Game");
			auth.msg_code=510;
		
			write(fd,&auth.msg_code,sizeof(int));
			//write(fd,&auth,sizeof(struct loginAuth));
			return 4;
			break;
	}
	return 0; // eState = 0 으로 로그인 다시 진행
}

int lobby(int epfd, int fd)
{
    int str_len;
	int i, rcnt, divrcnt, modrcnt;
	rcnt = inquiryRcount(); divrcnt = rcnt/4; modrcnt = rcnt%4;
	struct lobbyMsg lobbyMsg;
	struct lobbyRequest *request;
	struct lobbyAuth lobbyAuth;
	struct lobbyEnterAuth lobbyEnterAuth;
	struct lobbyListAuth lobbyListAuth;
	struct lobbyListAuth Listbuffer[divrcnt+1]; // 되는지 모름, 방 목록을 담기위한 임시 버퍼

	Log("Lobby module");
	LogNum("fd",fd);
	//sigPipe_handler(epfd,fd);

	str_len=read(fd,&lobbyMsg,sizeof(struct lobbyMsg)); // 로그인 단계 메시지를 받음

	switch(lobbyMsg.msg_code)
	{
		case 200:// 방 생성 요청
			//id, password 비교
			Log("request RoomCreate");
			request=(struct lobbyRequest *)&lobbyMsg;

			lobbyListAuth.msg_code=210;
			createRoom(request->sid,&lobbyListAuth.room[0]); //slot을 배정하는 배열을 만들어야 하는데 아직 정하지못함
			write(fd,&lobbyListAuth,sizeof(struct lobbyListAuth));
			return 2; // 방(inRoom) 상태로 전환
			break;
			
		case 201: // 방 접속 요청 , 방 번호를 전달 받으면 해당 방에 접속, 해당 방을 갱신한다.
			Log("request RoomEnter");
			request=(struct lobbyRequest *)&lobbyMsg; // 네명이 접속시 다 찼다는 예외처리 필요해보임

			
			if (enterRoom(request->rid, request->sid, &lobbyEnterAuth)) // rid가 존재하지 않을경우 실패
			{
				lobbyEnterAuth.msg_code=211;
				write(fd,&lobbyEnterAuth,sizeof(struct lobbyEnterAuth));
				return 2; // 방(inRoom) 상태로 전환
			}
			else // 방 접속 실패
			{
				lobbyEnterAuth.msg_code=214;
				write(fd,&lobbyEnterAuth.msg_code,sizeof(int));
			//	write(fd,&lobbyEnterAuth,sizeof(struct lobbyEnterAuth));
			}
			break;
		
		case 202: // 방 리스트 요청
			Log("request RoomList");
			request=(struct lobbyRequest *)&lobbyMsg;
		
			lobbyListAuth.msg_code=212;

			bringRoomList(&Listbuffer, divrcnt, modrcnt);

			for(i = 0; i < divrcnt; i++)
            { // 방 목록 전송 과정 1
                memcpy(&lobbyListAuth.room[0], &Listbuffer[i].room[0], sizeof(struct room));
                memcpy(&lobbyListAuth.room[1], &Listbuffer[i].room[1], sizeof(struct room));
                memcpy(&lobbyListAuth.room[2], &Listbuffer[i].room[2], sizeof(struct room));
                memcpy(&lobbyListAuth.room[3], &Listbuffer[i].room[3], sizeof(struct room));
                write(fd,&lobbyListAuth,sizeof(struct lobbyListAuth));
			}

			for (i = 0; i < modrcnt; i++) { // 방 목록 전송 과정 2 0~3개 남은 방들을 담아 전송
			memcpy(&lobbyListAuth.room[i], &Listbuffer[divrcnt].room[i], sizeof(struct room));
			}
			write(fd,&lobbyListAuth,sizeof(struct lobbyListAuth));
			break;
			
		case 203: // 방 개수 요청, lobby 전환시 클라이언트가 자동으로 신청함
			Log("request RoomCount");
			lobbyAuth.msg_code = 213;
			lobbyAuth.roomCount = rcnt;
			write(fd, &lobbyAuth, sizeof(struct lobbyAuth));
			
		case 500: // 게임 종료 요청, eState를 4로 바꿔야함
			Log("request Quit Game");
			lobbyListAuth.msg_code=510;
			write(fd,&lobbyListAuth.msg_code,sizeof(int));
			//write(fd,&lobbyListAuth,sizeof(struct lobbyListAuth));
			return 4;

			break;
	}
}

int inRoom(int epfd, int fd)
{
	return 4;
}

//server main code
int server(char *port)
{
	int i;

	int sock;
	int epfd;

	int event_cnt;
	struct epoll_event	*ep_events=(struct epoll_event *)malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

	void sigInt_handler();

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
				eState = login(epfd, eFD);
			}

			else if(eState == 1)//대기실 모듈일 때
			{
				eState = lobby(epfd, eFD);
			}

			else if(eState == 2)//방(inRoom) 모듈일 때
			{
				eState = inRoom(epfd, eFD);
			}
			/*
			else if(eState == 3)//인게임 모듈일 때
			{
				eState = (epfd, eFD);
			}
            */
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
