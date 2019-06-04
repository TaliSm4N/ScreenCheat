#ifndef UDP_DATA_H
#define UDP_DATA_H
#include <sys/socket.h>

typedef struct udpDataSet
{
	int sock;
	int clntCnt;
	struct sockaddr_in clnt_addr[4];

}udpDataSet;

#endif
