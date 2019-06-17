#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdlib.h>
#include "udpMsg.h"
#include "myBoolean.h"
#include "location.h"

void error_handling(char *);
void Log(char *);
void LogNum(char *,int);
void LogUDPMsg(udpMsg);
void LogLoc(loc);

#endif
