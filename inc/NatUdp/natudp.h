//
// Created by liubo on 2017/5/8.
//

#ifndef NATCOM_NATUDP_H
#define NATCOM_NATUDP_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <assert.h>
#include "thread"
#include<time.h>
#include<signal.h>
#include<errno.h>
#include<setjmp.h>

#ifdef _WIN32
#include <math.h>
#include <inaddr.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <fcntl.h>
#include <windows.h>
//#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <cmath>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include  <netinet/in.h>
#include <netinet/tcp.h>
#endif

#define prntlog(...) printf("%s[%d]: ",__FILE__,__LINE__);printf(__VA_ARGS__);//printf("\n")
#ifdef _WIN32
#define prnterr(...) printf("%s[%d]: ",__FILE__,__LINE__);printf(__VA_ARGS__);printf("err=%d.\n",WSAGetLastError())
#else
#define prnterr(...) printf("%s[%d]: ",__FILE__,__LINE__);printf(__VA_ARGS__);printf("err=%d.\n",errno)
#endif
typedef unsigned int uint32;
typedef unsigned short ushort;

const int MSGRQST=1;
const int MSGASK=2;
const int MSGANSWER=3;
const int MSGACKNOWLEDGE=4;
const int MSGREADY=5;
const int MSGERROR=6;

const ushort SVRPORT=9000;//udp server port, for access nats between diffirent LANs.
const char UDPSVRIP[]="123.56.142.186";// 172.16.187.100
bool bSvrRun;

struct natRequest
{//1
    int nMsgType;
    uint32 uDestIP;
    ushort uDestPort;
};

struct natAsk
{//2
    int nMsgType;
    uint32 uSrcIP;
    ushort uSrcPort;
    ushort uYourPort;
};

struct natAnswer
{//3
    int nMsgType;
    uint32 uInnerIP;
    ushort uInnerPort;
};

struct natAcknowledge
{//4
    int nMsgType;
    uint32 uOutterIP;
    ushort uOutterPort;
};

struct natReady
{//5
    int nMsgType;
};

struct natError
{//6
    int nMsgType;
    int nErr;//0: receiver login; 1: sender login
};

#endif //NATCOM_NATUDP_H
