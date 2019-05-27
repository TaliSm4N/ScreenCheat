#ifndef _INGAME_H
#define _INGAME_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "position.h"
#include "socketData.h"
#include "udpData.h"
#include "main.h"

#define EPOLL_SIZE 10

void *inGame_main(void *arg);
int UDPSet(int port, struct sockaddr_in *);
void *UDP_func(void *);

#endif
