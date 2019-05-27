#include "main.h"



//인게임 전 세팅해야할 것들 -> main에 다 때려박음 추후 수정
int main(int argc, char *argv[])
{
	int serv_sock;
	int clnt_sock[PLAYER_CNT];
	pthread_t t_id;
	pthread_t udp_t_id;
	socketData sockData;	

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;
	socklen_t clnt_addr_size;

	int udp_sock;
	struct sockaddr_in udp_clnt_adr[PLAYER_CNT];
	struct sockaddr_in udp_adr;
	socklen_t udp_clnt_sz;
	udpDataSet udpData;

	char msg[100];

	//서버 시작
	if(argc!=2)
	{
		fprintf(stderr,"Usage : %s <port>\n",argv[0]);
		exit(1);
	}

	serv_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(serv_sock == -1)
	{
		fprintf(stderr,"socket error\n");
		exit(1);
	}
	printf("TCP socket make\n");

	memset(&serv_addr, 0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr))==-1)
	{
		fprintf(stderr,"bind error\n");
		exit(1);
	}

	printf("TCP socket bind\n");

	if(listen(serv_sock, 5)==-1)
	{
		fprintf(stderr,"listen error\n");
		exit(1);
	}
	
	clnt_addr_size = sizeof(clnt_addr);
	
	//지정된 플레이어 수만큼 연결
	for(int i=0;i<PLAYER_CNT;i++)
	{
		clnt_sock[i]=accept(serv_sock,(struct sockaddr *) &clnt_addr, &clnt_addr_size);
		if(clnt_sock[i]==-1)
		{
			fprintf(stderr,"accept error");
			exit(1);
		}
		printf("make client %d\n",i);

		//read(clnt_sock[i],)
	}

	//시작메시지?? 넣을 것인가??

	//udp소켓 생성
	udp_sock=UDPSet(9191,&udp_adr);

	for(int i=0;i<PLAYER_CNT;i++)
	{
		udp_clnt_sz=sizeof(udp_clnt_adr[i]);
		//target UDP소켓 전송
		//C#에서의 udp adr의 방식에 따라서 변경해야함
//		write(clnt_sock[i],(char *)&udp_adr,sizeof(udp_adr));

		
		//target에 대응하는 UDP주소를 받음
		//방식의 변경의 가능성 있음
//		read(clnt_sock[i],(char *)&udp_adr,sizeof(udp_clnt_sock[i]));


		//udp socket의 포트 번호만 주고받는 방식
		write(clnt_sock[i],"9191",sizeof("9191"));

		//대답에 대해서 듣는 방식 결정할 필요있음
		//혹은 포트를 전해줬으니 알아서 들었다고 판단
		
		//사이즈는 임의 결정할 필요있음
		//udp의 연결을 확인하고 이에 대해 타겟에 대한 udp 소켓을 생성
		recvfrom(udp_sock,msg,100,0,(struct sockaddr*)&udp_clnt_adr[i],&udp_clnt_sz);

		printf("set client %d udp sock\n",i);
	}

	printf("set all client udp sock\n");

	//while(1);


	//tcp sock에 대한 thread 생성
	sockData.clnt_sock=clnt_sock;
	sockData.clntCnt=sizeof(clnt_sock)/sizeof(int);
	if(pthread_create(&t_id, NULL,inGame_main,(void *)&sockData)!=0)
	{
		fprintf(stderr,"pthread fail");
		exit(1);
	}


	//udp 데이터에 대한 sock생성
	udpData.sock=udp_sock;
	udpData.clntCnt=sizeof(clnt_sock)/sizeof(int);
	for(int i=0;i<PLAYER_CNT;i++)
	{
		udpData.clnt_addr[i]=udp_clnt_adr[i];
	}
	if(pthread_create(&udp_t_id,NULL,UDP_func,(void *)&udpData)!=0)
	{
		fprintf(stderr,"udp thread fail");
		exit(1);
	}

	//if(pthread_create(&udp_t_id,NULL,UDPSet)

	while(1);


	return 0;
}

