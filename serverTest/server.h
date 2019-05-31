#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include "log.h"
#include "msg.h"

#define EPOLL_SIZE 50

#define eFD ((eventData *)(ep_events[i].data.ptr))->fd
#define eState ((eventData *)(ep_events[i].data.ptr))->state

//epoll에 있는 데이터
typedef struct eventData
{
	int fd;
	int state;//어떤 모듈에 있는지 0 Login 모듈 1대기실 모듈 2 방모듈 3 인게임
	// 추가할 거 있으면 필요할 때 추가
}eventData;

int setListening(char *);
int setEpoll(int);
int connectClient(int,int);
int login(int);
int server(char *port);

#endif
