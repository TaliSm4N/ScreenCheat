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
#include "myBoolean.h"
#include "log.h"

void testIngame(int TPCport, int UDPport, int pCnt);//테스트 코드
int setUDP(int port);
int connectCheckUDP(int sock,struct sockaddr_in *clnt_adr,int cnt);
int playGame(int sock,struct sockaddr_in *clnt_adr,int cnt);

#endif
