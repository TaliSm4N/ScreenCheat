#ifndef INGAME_H
#define INGAME_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "udpMsg.h"
#include "tcpMsg.h"
#include "myBoolean.h"
#include "log.h"

#include <time.h>

#define EPOLL_SIZE 50
#define OBJ_CNT 15

#define MAP_CLNT 400
#define MAP_SERV 410
#define USR_CLNT 401
#define USR_SERV 411
#define ATK_CLNT 402
#define ATK_SERV 412
#define ITEM_CLNT 403
#define ITEM_SERV 413
#define UDPCHECK  414

struct tcpThreadArg
{
//	int epfd;
	int *player;
	int pCnt;
};

void testIngame(int TPCport, int UDPport, int pCnt);//테스트 코드

//tcp관련

int testTCP(int *,int,int);
int inGameEpoll(int *,int);
int initGame(int,int *,int);
void *tcp_thread(void *);

//udp관련
int setUDP(int port);
int connectCheckUDP(int sock,int *player,struct sockaddr_in *clnt_adr,int cnt);
int playGame(int sock,struct sockaddr_in *clnt_adr,int cnt);

#endif
