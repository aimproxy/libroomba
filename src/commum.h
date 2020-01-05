#ifndef __COMMUM_H__
#define __COMMUM_H__

// the usual suspects
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// socket includes
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>

// Json
#include <json.h>

// wolfSSL
#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

#define DISCOVERY_PORT  8883
#define BUFSIZE         1024

void error(const char *msg);
void discovery();
void getRobotInfo(const char* ip);
int getPassword(const char* host);

#endif
