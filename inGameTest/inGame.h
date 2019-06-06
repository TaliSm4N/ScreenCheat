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

struct tcpTheadArgment
{
};

void testIngame(int TPCport, int UDPport, int pCnt);//테스트 코드

//tcp관련

int testTCP(int *,int,int);
int inGameEpoll(int *,int);
int initGame(int,int *,int);

//udp관련
int setUDP(int port);
int connectCheckUDP(int sock,struct sockaddr_in *clnt_adr,int cnt);
int playGame(int sock,struct sockaddr_in *clnt_adr,int cnt);

#endif
