#include "server.h"

int SIGINTCount = 0; // sigint 발생시 처리하기 위해 만든 전역변수
int sigPipeCount = 0; // sigpipe 발생시 처리하기 위해 만든 전역변수

int udpPort = 9000;

//SIGINT 처리
void sigInt_handler (void)
{
    Log("sigInt_handler ");
	struct sigaction act_ignore;
	memset(&act_ignore, 0x00, sizeof(struct sigaction));

	act_ignore.sa_handler = sigintIgnore;
	sigemptyset(&(act_ignore.sa_mask));
	act_ignore.sa_flags = 0;

	sigaction(SIGINT, &act_ignore, NULL); //Ctrl-C

}

//SIGPIPE 처리, 종료되는 시그널이 발생한 경우 클라이언트의 소켓을 회수하고, epoll을 해제한다
void sigPipe_handler(void)
{
	struct sigaction act_ignore2;
	memset(&act_ignore2, 0x00, sizeof(struct sigaction));

	act_ignore2.sa_handler = sigPipeIgnore;

	sigemptyset(&(act_ignore2.sa_mask));
	act_ignore2.sa_flags = 0;

	sigaction(SIGPIPE, &act_ignore2, NULL);

}

// SIGINT 발생시 무시한다
void sigintIgnore(int signo)
{
	Log("sigintIgnored");
	SIGINTCount = 1;
}

// 시그파이프를 발생시킨 소켓을 종료시킨다
void sigPipeIgnore(int signo)
{
	Log("sigPipeIgnored");
	sigPipeCount = 1;
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
	setnonblockingmode(sock); // 비동기화lobbyCreateAuth.msg_code=210
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
}

//login module msg처리
int login(int epfd ,int fd)
{
	if (sigPipeCount == 1) { //SIGPIPE 발생시
		Log("SIGPIPE occured");
		sigPipeCount = 0;
		closeClient(epfd, fd);
	}

	int str_len;
	struct loginMsg loginMsg;
	struct loginRequest *request;
	struct loginAuth auth;

	Log("Login module");
	LogNum("fd",fd);

	while(1)
	{
		str_len=read(fd,&loginMsg,sizeof(struct loginMsg)); // 로그인 단계 메시지를 받음
		if(str_len != -1)
			break;
	}
	switch(loginMsg.msg_code)
	{
		case 100:// 로그인 요청
			//id, password 비교
			Log("request Login");
			request=(struct loginRequest *)&loginMsg;

			Log("id");



			if (compareID (request->id, request->pwd, &auth.profile, &auth.uid))
			{
				Log("success Login");
				auth.msg_code=110;
				write(fd,&auth,sizeof(struct loginAuth)); // 로그인에 성공한 클라이언트 정보 전송
			}

			else {
				Log("failed Login");
				auth.msg_code=114;
				write(fd,&auth.msg_code,sizeof(auth.msg_code)); // 실패 했으므로 uid -1 전송
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
			break;

		case 102: // 대기실 상태로 전환한다.
			Log("request change to Lobby");
			auth.msg_code=112;
			write(fd,&auth.msg_code,sizeof(int));
			return 1; // 대기실 모듈로 전환
			break;

		case 500: // 게임 종료 요청, eState를 4로 바꿔야함
			Log("request Quit Game");
			auth.msg_code=510;

			write(fd,&auth.msg_code,sizeof(int));
			return 4;
			break;
	}
	return 0; // eState = 0 으로 로그인 다시 진행
}

//lobby module msg처리
int lobby(int epfd, int fd, int *stats)
{
	if (sigPipeCount == 1) { //SIGPIPE 발생시
		Log("SIGPIPE occured");
		sigPipeCount = 0;
		closeClient(epfd, fd);
	}

    int str_len;
	int i, tempI, divrcnt, modrcnt;
	int roomCount = inquiryCount("RoomList"); // 전체 방 수
    divrcnt = roomCount/4; modrcnt = roomCount%4; // 방 목록을 가져오기 위해 설정한 변수
	int broadcastfd[4] = {0}; // broadcast용 fd
	int enterRid, enterUid; // 입장할 때 임시로 저장해 놓기위한 변수
	int hostfd; // 방장의 fd

	struct buffer { // write 한 방에 다 보내기위해 만든 구조체
		int msg_code;
		struct room room[roomCount + 1];
	};
	struct buffer buffer;

	struct lobbyMsg lobbyMsg;
	struct lobbyRequest *request;
	struct lobbyStatsFromHost StatsFromHost;
	struct lobbyAuth lobbyAuth;
	struct lobbyCreateAuth lobbyCreateAuth;
	struct lobbyEnterAuth lobbyEnterAuth;
	struct lobbyListAuth lobbyListAuth;
	struct lobbyListAuth_2 Listbuffer[divrcnt+1]; //방 목록을 담기위한 임시 버퍼, 나중에 struct room으로 교체하거나 내비둠

	struct user updateUser;
	struct enterRoomBroadcast enterRoomBroadcast;    LogNum("roomCount",roomCount);

	Log("Lobby module");
	LogNum("fd",fd);

	while(1)
	{
		str_len=read(fd,&lobbyMsg,sizeof(struct lobbyMsg)); // 로그인 단계 메시지를 받음
		if(str_len != -1)
			break;
	}

	switch(lobbyMsg.msg_code)
	{
		case 200:// 방 생성 요청
			//id, password 비교
			Log("request RoomCreate");
			request=(struct lobbyRequest *)&lobbyMsg;

			lobbyCreateAuth.msg_code=210;
			createRoom(request->uid, fd, &lobbyCreateAuth);

            LogNum("lobbyCreateAuth rid", lobbyCreateAuth.rid);
			write(fd,&lobbyCreateAuth,sizeof(struct lobbyCreateAuth));

			stats[0] = 0; // 방 생성후 상태 변경
            // 만약 states 관리가 꼬인다면 return을 하지않고 직접 inRoom 함수를 호출하는 방식으로 진행해야함
			return 2; // 방(inRoom) 상태로 전환
			break;

		case 201: // 방 접속 요청 , 방 번호를 전달 받으면 해당 방에 접속, 해당 방을 갱신한다.
			Log("request RoomEnter");
			request=(struct lobbyRequest *)&lobbyMsg;

			//호스트로부터 먼저 stats를 요청하는 과정
			LogNum("msg201 rid ", request->rid);
			hostfd = inquiryHostfd(request->rid); // 호스트의 fd를 가져옴

			LogNum("hostfd", hostfd);

			lobbyEnterAuth.msg_code = 215;
			write(hostfd, &lobbyEnterAuth.msg_code, sizeof(int)); // 호스트에게 stats 달라고 요청
			while(1)
			{
				str_len = read(hostfd, &StatsFromHost ,sizeof(struct lobbyStatsFromHost)); // 호스트로부터 메시지 받아옴
				if(str_len != -1)
					break;
			}
			LogNum("Host message code", StatsFromHost.msg_code); // 어떤 메세지코드를 받는지 검사
			if(StatsFromHost.msg_code == 204) { // 204 메시지 코드일때만 수행되어야 함
			memcpy(stats, StatsFromHost.stats, sizeof(stats)); // stats 복사
			printf("stats: %d %d %d %d \n", stats[0], stats[1], stats[2], stats[3]);
			}
			else { // 호스트의 메세지코드가 이상할 경우 임시로 방 접속 실패 처리로 해둠
				Log("client enter room failed");
				lobbyEnterAuth.msg_code=214;
				write(fd,&lobbyEnterAuth.msg_code,sizeof(int));
				LogNum("fd", fd);
				break;
			}
			// stats 과정 종료
			LogNum("msg201 rid ", request->rid);
            LogNum("msg201 uid", request->uid);
			if (enterRoom(request->rid, request->uid, fd, stats, broadcastfd, &lobbyEnterAuth)) // rid가 존재하지 않을경우 실패
			{
				lobbyEnterAuth.msg_code = 211;
				write(fd,&lobbyEnterAuth,sizeof(struct lobbyEnterAuth)); // 다른 유저들의 정보, 자기 슬롯, 방의 유저 수 전달
				LogNum("enterUser slot(lobbyEnterAuth)", lobbyEnterAuth.slot);
                printf("user0: %s \n", lobbyEnterAuth.user[0].id);
				printf("user1: %s \n", lobbyEnterAuth.user[1].id);
				printf("user2: %s \n", lobbyEnterAuth.user[2].id);
				enterRoomBroadcast.msg_code=321;
				bringUserinfo(request->uid, &updateUser); // profile 가져옴

				memcpy(&enterRoomBroadcast.user, &updateUser, sizeof(updateUser)); // profile 담음
				printf("enterUser id : %s, win: %d, lose: %d \n", enterRoomBroadcast.user.id, enterRoomBroadcast.user.win, enterRoomBroadcast.user.lose);
				enterRoomBroadcast.user.slot = lobbyEnterAuth.slot; // 입장할 유저가 몇번 슬롯에 갈지 받아옴
				LogNum("enterUser slot", enterRoomBroadcast.user.slot);
				enterRoomBroadcast.user.stats = 0; // 입장 했으므로 0

				for(i = 0; i < 4; i++) // guest들에게 들어온 유저에 대한 정보를 전달
				{
					if(broadcastfd[i] == fd || broadcastfd[i] == 0) // 없는 유저나 자신은 그냥 넘어감
						continue;

					write(broadcastfd[i],&enterRoomBroadcast,sizeof(struct enterRoomBroadcast));
					printf("broadcastfd[%d] = %d\n", i, broadcastfd[i]);

				}
				 // 만약 states 관리가 꼬인다면 return을 하지않고 직접 inRoom 함수를 호출하는 방식으로 진행해야함
				return 2; // 방(inRoom) 상태로 전환
			}

			else // 방 접속 실패
			{
				Log("client enter room failed");
				lobbyEnterAuth.msg_code=214;
				write(fd,&lobbyEnterAuth.msg_code,sizeof(int));
				LogNum("fd", fd);
			}

			break;

		case 202: // 방 리스트 요청
			Log("request RoomList");

			lobbyListAuth.msg_code=212;

			bringRoomList(Listbuffer, divrcnt, modrcnt);

			for(i = 0; i < divrcnt; i++)
            { // 방 목록 전송 과정 1
				tempI = i*4;

                memcpy(&buffer.room[0+tempI], &Listbuffer[i].room[0], sizeof(struct room));
                memcpy(&buffer.room[1+tempI], &Listbuffer[i].room[1], sizeof(struct room));
                memcpy(&buffer.room[2+tempI], &Listbuffer[i].room[2], sizeof(struct room));
                memcpy(&buffer.room[3+tempI], &Listbuffer[i].room[3], sizeof(struct room));
			}

            for (i = 0; i < modrcnt; i++)
            { // 방 목록 전송 과정 2 0~3개 남은 방들을 담아 전송
			memcpy(&buffer.room[(divrcnt*4)+i], &Listbuffer[divrcnt].room[i], sizeof(struct room));
			}

			memset(&buffer.room[roomCount], -1, sizeof(struct room));
			write(fd, &buffer, sizeof(buffer));
			break;

		case 500: // 게임 종료 요청, eState를 4로 바꿔야함
			Log("request Quit Game");
			lobbyListAuth.msg_code=510;
			write(fd,&lobbyListAuth.msg_code,sizeof(int));
			return 4;

			break;
	}
	return 1; // eState = 1 으로 대기실 다시 진행
}

int inRoom(int epfd, int fd, int *stats)
{
	if (sigPipeCount == 1) { //SIGPIPE 발생시
		Log("SIGPIPE occured");
		sigPipeCount = 0;
		closeClient(epfd, fd);
	}

    int str_len;
    int i;
    int broadcastfd[4] = {0}; // broadcast용 fd

    pthread_t t_id;

    struct gameArg gameArg;
    struct inRoomMsg inRoomMsg;
	struct inRoomRequest *request;
	struct inRoomStartAuth inRoomStartAuth;
	struct inRoomStateBroadcast inRoomStateBroadcast;
    struct inRoomStateBroadcast temp;

    Log("inRoom module");
	LogNum("fd",fd);

	while(1)
	{
		str_len=read(fd,&inRoomMsg,sizeof(struct inRoomMsg)); // 로그인 단계 메시지를 받음
		if(str_len != -1)
			break;
	}
    request=(struct inRoomRequest *)&inRoomMsg;

	switch(inRoomMsg.msg_code)
	{
        case 300: // 상태 변경 요청
            Log("request User Status Change");
            inRoomStateBroadcast.msg_code=310;
            write(fd,&inRoomStateBroadcast.msg_code,sizeof(int)); // 신청 대상에게 잘 받았다고 먼저 전송

            broadcastInRoom(fd, request->rid, broadcastfd, &inRoomStateBroadcast);
			LogNum("User slot", inRoomStateBroadcast.slot);
            inRoomStateBroadcast.msg_code=320;
			for(i = 0; i < 4; i++) {
				if(broadcastfd[i] == fd) // 요청한 유저의 상태만 기록시킨다
					{
						if (stats[i] == 0) {
							stats[i] = 1;
							inRoomStateBroadcast.stats = 1;
						}
						else if (stats[i] == 1) {
							stats[i] = 0;
							inRoomStateBroadcast.stats = 0;
						}
					}
			}
            for(i = 0; i < 4; i++) // 다른 유저들에게 변경된 상태를 전달
				{
					if(broadcastfd[i] == fd || broadcastfd[i] == 0) // 없는 유저나 자신은 그냥 넘어감
					{
					  continue;
					}
					write(broadcastfd[i], &inRoomStateBroadcast, sizeof(struct inRoomStateBroadcast));
				}
			printf("bcfd = %d %d %d %d\n", broadcastfd[0], broadcastfd[1], broadcastfd[2], broadcastfd[3]);
			LogNum("changedUser stats", inRoomStateBroadcast.stats);
            break;

        case 301: // 게임 시작 요청, 그 방에 있는 모든 유저에게 승인 메시지 전송
            Log("request Game Start");
            inRoomStartAuth.msg_code=311;
            // fd를 받아오기 위함 , broadcastfd와 rid 말곤 필요없는 정보, temp는 그냥 형식상 넘김
            broadcastInRoom(fd, request->rid, broadcastfd, &temp);
            inRoomStartAuth.portNum = udpPort;
            for(i = 0; i < 4; i++) // 모두에게 311번(게임이 시작) 코드 전달
				{
					write(broadcastfd[i],&inRoomStartAuth.msg_code,sizeof(int));
				}
			printf("bcfd = %d %d %d %d\n", broadcastfd[0], broadcastfd[1], broadcastfd[2], broadcastfd[3]);

			//epoll에서 뺴줌
			for(i=0;i<4;i++)
			{
				epoll_ctl(epfd,EPOLL_CTL_DEL,broadcastfd[i],NULL);
			}
			//thread arg세팅
			gameArg.UDP=udpPort;
			udpPort++;
			gameArg.pCnt=0;
			for(i=0;i<4;i++)
			{
				if(broadcastfd[i]!=0)
					gameArg.pCnt++;
			}
			gameArg.player=broadcastfd;

			//게임 시작 thread 실행
			if(pthread_create(&t_id,NULL,inGameStart,&gameArg)!=0)
			{
				error_handling("inGame start error");
			}

			//수정 끝인 부분


			/*
            inRoomStartAuth 에 포트 넘버 넘기는 과정 필요

            여기서 broadcastfd[i] 들을 전부 epoll 해제 시키고 넘겨주는 과정이 필요하다
            epoll_ctl(epfd, EPOLL_CTL_DEL, broadcastfd[i], NULL); // epoll 관리 대상에서 제외

			게임 종류 이후 방으로 복귀할 경우 stats[i] = 0;(존재하는 클라것만)
            */
            return 3; // 인게임 모듈로 전환
            break;

        case 302: // 방 퇴장 요청
            Log("request Exit Room");
            inRoomStateBroadcast.msg_code=312;
            write(fd,&inRoomStateBroadcast.msg_code,sizeof(int)); // 신청 대상에게 잘 받았다고 먼저 전송
            LogNum("msg302 rid ", request->rid);
            broadcastInRoom(fd, request->rid, broadcastfd, &inRoomStateBroadcast);
            inRoomStateBroadcast.msg_code=320;
			inRoomStateBroadcast.stats = -1;
            for(i = 0; i < 4; i++) // 다른 유저들에게 변경된 상태를 전달
				{
					if(broadcastfd[i] == fd || broadcastfd[i] == 0) // 없는 유저나 자신은 그냥 넘어감
					{
					    stats[i] = -1; // 없는 유저나 퇴장한 유저는 똑같이 -1 이므로 그냥 진행
					    continue;
					}
					write(broadcastfd[i], &inRoomStateBroadcast, sizeof(struct inRoomStateBroadcast));
				}
			printf("bcfd = %d %d %d %d\n", broadcastfd[0], broadcastfd[1], broadcastfd[2], broadcastfd[3]);
			LogNum("ExitedUser slot", inRoomStateBroadcast.slot);
			exitRoom(fd, request->rid, broadcastfd);
            return 1; // 대기실 모듈로 전환
            break;

        case 303: // 방장의 방 퇴장 요청
            Log("request Delete Room");
            inRoomStateBroadcast.msg_code=313;

            LogNum("msg303 rid ", request->rid);
            broadcastInRoom(fd, request->rid, broadcastfd, &inRoomStateBroadcast);
            printf("broadcastfd: %d %d %d %d\n",broadcastfd[0],broadcastfd[1],broadcastfd[2],broadcastfd[3]);
            for(i = 0; i < 4; i++) // 모든 유저들에게 변경된 상태를 전달
				{
					if(broadcastfd[i] == 0) // 없는 유저는 그냥 넘어감
						continue;

					stats[i] = -1; // 모두 나갔으므로 전부 -1로 초기화
					write(broadcastfd[i], &inRoomStateBroadcast.msg_code, sizeof(int));
				}

			deleteRoom(request->rid); // 방을 삭제한다

            break;

        case 304: // 방장의 방 퇴장 요청에 따른 클라이언트 들의 대기실 이동
            Log("request return lobby");
            return 1; // 대기실 모듈로 전환
            break;

        case 500: // 게임 종료 요청, eState를 4로 바꿔야함
			Log("request Quit Game");
			inRoomStartAuth.msg_code=510;
			write(fd,&inRoomStartAuth.msg_code,sizeof(int));

			broadcastInRoom(fd, request->rid, broadcastfd, &inRoomStateBroadcast);
            inRoomStateBroadcast.msg_code=320;
            for(i = 0; i < 4; i++) // 다른 유저들에게 변경된 상태를 전달
				{
					if(broadcastfd[i] == fd || broadcastfd[i] == 0) // 없는 유저나 자신은 그냥 넘어감
					{
					    stats[i] = -1;
						inRoomStateBroadcast.stats = -1;
					    continue;
					}
					write(broadcastfd[i],&inRoomStateBroadcast,sizeof(struct inRoomStateBroadcast));
				}
			printf("bcfd = %d %d %d %d\n", broadcastfd[0], broadcastfd[1], broadcastfd[2], broadcastfd[3]);
			return 4;

			break;
	}
	return 2; // eState = 2 으로 방(inRoom) 다시 진행
}

//server main code
int server(char *port)
{
	int i;
    int stats[4] = {-1,-1,-1,-1}; // 유저들 상태를 관리
	int sock;
	int epfd;

	int event_cnt;
	struct epoll_event	*ep_events=(struct epoll_event *)malloc(sizeof(struct epoll_event)*EPOLL_SIZE);

	sigInt_handler();
    sigPipe_handler();

	sock=setListening(port);//3
	epfd=setEpoll(sock);//4
	connectDB();
	Log("server");

	while(1)
	{
		event_cnt=epoll_wait(epfd,ep_events,EPOLL_SIZE,-1);
		if(event_cnt == -1)
		{
		    if (SIGINTCount == 1) // SIGINT가 발생한 상황
            {
                SIGINTCount = 0;
                continue;
            }
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
				eState = lobby(epfd, eFD, stats);
			}

			else if(eState == 2)//방(inRoom) 모듈일 때
			{
				eState = inRoom(epfd, eFD, stats);
			}
			/*
			else if(eState == 3)//인게임 모듈일 때
			{
				eState = (epfd, eFD, stats);
				// stats는 게임중에 사용하지 않으나 게임이 끝날 때 방으로 다시 돌아가는 시나리오면 전부 0,0,0,0 으로 만들어 줘야한다
			}
            */
			else if(eState == 4) // 종료 요청
			{
				closeClient(epfd, eFD);
			}
		}
	}

	closeDB();
	close(epfd);

	return 0;
}
