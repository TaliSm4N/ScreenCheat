#ifndef SERVER_H
#define SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <signal.h>
#include <fcntl.h>
#include "log.h"
#include "msg.h"
#include "database.h"

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

void sigInt_handler(void); //SIGINT 처리
void sigPipe_handler(void); //SIGPIPE 처리, 종료되는 시그널이 발생한 경우 클라이언트의 소켓을 회수하고, epoll을 해제한다
void sigintIgnore(int); //SIGINT 발생시 Log를 남기고 무시함
void sigPipeIgnore(int); //SIGPIPE 발생시 소켓과 epoll을 해제하도록 전역변수의 값을 변경함
void setnonblockingmode(int); //비동기 소켓으로 바꿈
int setListening(char *);
int setEpoll(int);
int connectClient(int, int);
void closeClient(int, int);
int login(int, int);
int lobby(int, int, int *);
int inRoom(int, int, int *);
int server(char *port);

#endif
