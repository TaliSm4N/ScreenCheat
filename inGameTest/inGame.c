#include "inGame.h"

void testIngame(int TCPport, int UDPport,int pCnt)
{
	int udp_sock;

	struct sockaddr_in clnt_adr[pCnt];
	//tcp연계부분 제외


	udp_sock=setUDP(UDPport);

	//tcp를 통해 udp의 소켓 번호 전달(테스트용 생략)

	connectCheckUDP(udp_sock,clnt_adr,pCnt);
	playGame(udp_sock,clnt_adr,pCnt);

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

	for(int i=0;i<cnt;i++)
	{
		clnt_adr_sz=sizeof(clnt_adr[i]);
		recvfrom(sock,(char *)&msg,sizeof(udpMsg),0,
				(struct sockaddr*)&(clnt_adr[i]),&clnt_adr_sz);

		msg.id = i;

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
