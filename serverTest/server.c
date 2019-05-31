#include "server.h"

//listening socke setting
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
	event.events = EPOLLIN;
	eData->fd=sock;
	eData->state=0;
	event.data.ptr = (void *)eData;

	epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &event);

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
	event.events=EPOLLIN;
	eData->fd=clnt_sock;
	eData->state=0;
	event.data.ptr=(void *)eData;

	epoll_ctl(epfd,EPOLL_CTL_ADD,clnt_sock,&event);
	
	Log("client connect");
	LogNum("client fd",clnt_sock);

	return clnt_sock;
}

//login module msg처리
int login(int fd)
{
	int str_len; 

	struct loginMsg loginMsg;
	struct loginRequest *request;
	struct loginAuth auth;


	//test code
	//----------------------
	struct profile testID;//실험용 데이터 셋
	char pwd[20]="testpwd";

	strcpy(testID.id,"testid");//="testid";
	testID.win=10;
	testID.lose=8;
	testID.kill=32;
	testID.death=22;
	//-----------------------

	Log("Login module");
	LogNum("fd",fd);

	str_len=read(fd,&loginMsg,sizeof(struct loginMsg));




	switch(loginMsg.msg_code)
	{
		case 100://로그인 요청
			//id, password 비교
			Log("request Login");
			request=(struct loginRequest *)&loginMsg;

			Log("id");

			auth.msg_code=110;

			if(strcmp(request->id,testID.id)==0&&strcmp(request->pwd,pwd)==0)
			{
				auth.sid=1;
				strcpy(auth.profile.id,testID.id);
				auth.profile.win=testID.win;
				auth.profile.lose=testID.lose;
				auth.profile.kill=testID.kill;
				auth.profile.death=testID.death;

				Log("success Login");
				Log(request->id);
				
			}
			else
			{
				auth.sid=-1;

				Log("failed Login");
			}
			write(fd,&auth,sizeof(struct loginAuth));
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


	sock=setListening(port);//3
	epfd=setEpoll(sock);//4

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
				connectClient(sock,epfd);
			}
			else if(eState==0)//로그인 모듈일 때
			{
				login(eFD);
			}
		}
		
	}

	close(epfd);

	return 0;
}
