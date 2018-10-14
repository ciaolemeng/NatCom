//#include <iostream>
#include "NatUdp/natudp.h"

//using namespace std;
void thrdSvr();

void delayMs(int nMs)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(nMs));
}

bool operator==(sockaddr_in a,sockaddr_in b)
{
    if(a.sin_port==b.sin_port&&memcmp(&a.sin_addr,&b.sin_addr, sizeof(b.sin_addr))==0)
    {//&&a.sin_addr.S_un==b.sin_addr.S_un
        return true;
    }
    return false;
}

int createUdpskt(unsigned short uPort)
{
    int skt=-1,ret;
    skt= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if ((skt) == -1)
    {
        prnterr("UdpSock created failed!");
        return -1;
    }

    int ttl=128;
    ret=setsockopt(skt,IPPROTO_IP,IP_TTL,(const char*)&ttl, sizeof(ttl));
    assert(ret==0);

    int reUse=1;
    ret=setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, (char *)&reUse, sizeof(reUse));
    assert(ret==0);

    sockaddr_in	addr;
    memset(&addr,0, sizeof(addr));
    addr.sin_family=AF_INET;prntlog("%d\n",uPort);
    //   addr.sin_addr.s_addr = htonl(INADDR_ANY);//bind to any net card
    addr.sin_port=htons(uPort);//bind to a certain port 127.0.0.1
    ret = bind(skt, (const struct sockaddr *)&addr, sizeof(struct sockaddr_in));
    assert(ret==0);

    return skt;
}

int sktNoneBlock(int skt)
{
    int ret=0;
#ifdef _WIN32
    unsigned long ul = 1;
    ret=ioctlsocket(skt, FIONBIO, &ul);
    if(0 != ret)
    {
        prnterr("ioctlsocket FIONBIO.");
        close(skt);
        ret = -1;
    }
    prntlog("skt=%d.\n",skt);
#else
    int flags = fcntl(skt, F_GETFL, 0);assert(flags>0);
    if (flags < 0)
    {
        flags = 0;
    }
    ret = fcntl(skt, F_SETFL, flags | O_NONBLOCK);
    if(ret<0)
    {
        prnterr("ioctlsocket FIONBIO.\n");
        close(skt);
        ret = -1;
    }
    prntlog("skt=%d.\n",skt);
#endif
    return ret;
}

int rcvNoneblock(int skt, char *szbuf,int len,struct sockaddr_in &addr,int nWaitSec=5)
{
    struct timeval tv;
    fd_set rds;
    int ret;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    FD_ZERO(&rds);
    FD_SET(skt, &rds);
    tv.tv_sec = nWaitSec;//1
    tv.tv_usec = 20000;//wait 20ms for receiving

    ret=select(skt+1, &rds, 0, 0, &tv);
    if(0==ret)//timeout
    {
//        prnterr("recv timeout.\n");
        return 0;
    }
    if(ret<0)
    {
        prnterr("rcvNoneblock ret=%d.\n",ret);
        return ret;
    }
    memset(&addr,0, sizeof(addr));
    ret = recvfrom(skt, szbuf, len, 0, (struct sockaddr*)&addr, (socklen_t*)&addrlen);
    prnterr("noneblock rcv %dB from %s:%d.\n",ret,inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
    return ret;
}

int sndNoneblock(int skt, char *szbuf,int len,struct sockaddr_in &addr)
{
    struct timeval tv;
    fd_set wts;
    int ret;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    FD_ZERO(&wts);
    FD_SET(skt, &wts);
    tv.tv_sec = 0;//
    tv.tv_usec = 10000;//wait 10ms for sending

    ret=select(skt+1, 0, &wts, 0, &tv);
    if(0==ret)//timeout
    {
        prnterr("send timeout.\n");
        return 0;
    }
    if(ret<0)
    {
        prnterr("sndNoneblock ret=%d.\n",ret);
        return ret;
    }
    ret = sendto(skt, szbuf, len, 0, (struct sockaddr*)&addr, addrlen);
    prntlog("noneblock snd out %dB.\n",ret);
    return ret;
}

bool setNetaddr(sockaddr_in &addr,const char* szIP,ushort ushPort)
{
    memset(&addr,0, sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(ushPort);

#ifdef _WIN32
    addr.sin_addr.S_un.S_addr=inet_addr(szIP);
#else
    int ret = inet_aton((const char*)szIP, &addr.sin_addr);
    if(ret == 0)
    {
        prnterr("Init udp socket: inet_aton()." );
        return false;
    }
#endif
    return true;
}

void uint2str(uint32 u,char *szIP)
{
    unsigned char dm[4];
    dm[3]=u>>24;    dm[2]=u>>16;
    dm[1]=u>>8; dm[0]=u&0xff;
    sprintf(szIP,"%d.%d.%d.%d",dm[0],dm[1],dm[2],dm[3]);
}

uint32 str2uint(char *szIP)
{
    uint32 u=inet_addr(szIP);
    return u;
}

void thrdSndr(/*const char* szDestIP,ushort ushDestPort,*/ushort ushLocalPort)
{
    bool b;
    int ret,udpSndr,i=0;
    char szbuf[1024];
    sockaddr_in addrLocal,addrRemote,addrSvr,addr;
    natError natErr;
    natRequest natRqst;
    natReady natRdy;

    b=setNetaddr(addrLocal,"0.0.0.0",ushLocalPort);//local address
    assert(b);
    b=setNetaddr(addrSvr,UDPSVRIP,SVRPORT);//server address
    assert(b);

    udpSndr=createUdpskt(ushLocalPort);//create sender's socket and specify it as a none block one
    assert(udpSndr>0);
    ret=sktNoneBlock(udpSndr);
    assert(ret==0);

    natErr.nMsgType=MSGERROR;
    natErr.nErr=1;//sender login. try to get the receiver's public address
    ret=sndNoneblock(udpSndr,(char*)&natErr, sizeof(natErr),addrSvr);//notify server that i've logged in
    assert(ret==sizeof(natErr));

    ret=rcvNoneblock(udpSndr,szbuf,1024,addr);
    assert(ret== sizeof(natRequest));
    memcpy(&natRqst,szbuf,ret);
    uint2str(natRqst.uDestIP,szbuf);
    b=setNetaddr(addrRemote,szbuf,natRqst.uDestPort);//destination address of the receiver
    assert(b);

    /*ret=sndNoneblock(udpSndr,(char*)&natRqst, sizeof(natRqst),addrSvr);//notify server i want to communicate with the receiver
    assert(ret==sizeof(natRqst));*/
    natRdy.nMsgType=MSGREADY;
    ret=sndNoneblock(udpSndr,(char*)&natRdy, sizeof(natRdy),addrSvr);
    assert(ret==sizeof(natRdy));
    i=0;
    while(bSvrRun)
    {
        sprintf(szbuf,"send out %d to %s:%d.",i++,inet_ntoa(addrRemote.sin_addr),ntohs(addrRemote.sin_port));
        ret=sndNoneblock(udpSndr,szbuf, strlen(szbuf)+1,addrRemote);
        assert(ret==strlen(szbuf)+1);
        delayMs(1000);
    }
}

void thrdRcvr(ushort ushLocalPort)
{
    bool b;
    int ret,udpRcvr,nMsg;
    char szbuf[1024];
    sockaddr_in addrLocal,addrRemote,addrSvr,addr;
    natError natErr;
    natAsk natask;
    natReady natRdy;

    b=setNetaddr(addrLocal,"0.0.0.0",ushLocalPort);//local address
    assert(b);
    b=setNetaddr(addrSvr,UDPSVRIP,SVRPORT);//server address
    assert(b);

    udpRcvr=createUdpskt(ushLocalPort);
    assert(udpRcvr>0);
    ret=sktNoneBlock(udpRcvr);
    assert(ret==0);

    natErr.nMsgType=MSGERROR;
    natErr.nErr=0;//receiver login, not an error
    ret=sndNoneblock(udpRcvr,(char*)&natErr, sizeof(natErr),addrSvr);//notify server that i've logged in
    assert(ret==sizeof(natErr));

    while (1)
    {
        ret=rcvNoneblock(udpRcvr,szbuf,1024,addr);
        if(ret<=0)
        {
            continue;
        }

        memcpy(&nMsg,szbuf, sizeof(nMsg));//retrieve message type
        if(nMsg==MSGASK)
        {
            assert(ret== sizeof(natask));
            memcpy(&natask,szbuf,ret);
            uint2str(natask.uSrcIP,szbuf);
            prntlog("sender address: %s:%d.\n",szbuf,natask.uSrcPort);
            b=setNetaddr(addrRemote,szbuf,natask.uSrcPort);//remote address
            assert(b);

            prntlog("send random data to %s:%d.\n",inet_ntoa(addrRemote.sin_addr),ntohs(addrRemote.sin_port));
            ret=sndNoneblock(udpRcvr,(char*)&natRdy, sizeof(natRdy),addrRemote);//send random data to sender. mostly it will be refused by the remote NAT
            assert(ret==sizeof(natRdy));

            natRdy.nMsgType=MSGREADY;
            ret=sndNoneblock(udpRcvr,(char*)&natRdy, sizeof(natRdy),addrSvr);
            assert(ret==sizeof(natRdy));
            prntlog("wait ready signal from server.\n");
        }
        else if(nMsg==MSGREADY)
        {//it must be sent from server
            assert(addr==addrSvr);
            prntlog("server notify me the sender is ready.\n");
            prntlog("prepare to recv from %s:%d.",inet_ntoa(addrRemote.sin_addr),ntohs(addrRemote.sin_port));
            break;
        }
        else
        {
            prntlog("recv from %s:%d.",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
        }
    }

    int i=0;
    while (1)
    {
        sprintf(szbuf,"send out %d to %s%d.",i++,inet_ntoa(addrRemote.sin_addr),ntohs(addrRemote.sin_port));
        ret=sndNoneblock(udpRcvr,szbuf, strlen(szbuf)+1,addrRemote);

        ret = rcvNoneblock(udpRcvr, szbuf, 1024, addr);
        if (ret <= 0)
        {
            continue;
        }
        prntlog("rcv %dB form %s:%d, recv data: %s\n",ret,inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),szbuf);
    }
}

void thrdSvr()
{//only service for 2 LANs
    int udpSvr,ret,nMsg;
    natRequest natRqst;
    natAsk natask;
    natError natErr;
    natReady natRdy;
    char szbuf[1024];
    sockaddr_in addrSndr,addrRcvr,addr;

    udpSvr=createUdpskt(SVRPORT);
    assert(udpSvr>0);
    ret=sktNoneBlock(udpSvr);
    assert(ret==0);

    /*natAsk.nMsgType=MSGASK;
    natAsk.uSrcIP=natRqst.uDestIP;*/

    while(bSvrRun)
    {
        ret=rcvNoneblock(udpSvr,szbuf,1024,addr);
        if(ret<=0)
        {
            continue;
        }

        assert(ret>= sizeof(nMsg));
        memcpy(&nMsg,szbuf, sizeof(nMsg));//retrieve message type

        if(nMsg== MSGERROR)//receiver: 0, sender: 1
        {
            assert(ret== sizeof(natError));

            memcpy(&natErr,szbuf,ret);
            assert(natErr.nMsgType==MSGERROR);
            if(natErr.nErr==0)//receiver login
            {
                memcpy(&addrRcvr,&addr, sizeof(addr));
            }
            else if(natErr.nErr==1)//sender login
            {
                memcpy(&addrSndr,&addr, sizeof(addr));
                natRqst.nMsgType=MSGRQST;
                natRqst.uDestIP=str2uint(inet_ntoa(addrRcvr.sin_addr));
                natRqst.uDestPort=ntohs(addrRcvr.sin_port);//send receiver's address to sender
                ret=sndNoneblock(udpSvr,(char*)&natRqst, sizeof(natRqst),addr);
                assert(ret==sizeof(natRqst));

                natask.nMsgType=MSGASK;
                natask.uSrcIP=str2uint(inet_ntoa(addrSndr.sin_addr));
                natask.uSrcPort=ntohs(addrSndr.sin_port);//send sender's addresss to receiver
                ret=sndNoneblock(udpSvr,(char*)&natask, sizeof(natAsk),addrRcvr);
                assert(ret==sizeof(natAsk));
            }
            else
            {
                assert(0);
            }
        }
        else if(nMsg==MSGRQST)//from sender
        {
            assert(ret== sizeof(natRequest));
            assert(addr==addrSndr);
            memcpy(&natRqst,szbuf, sizeof(natRqst));
            natask.nMsgType=MSGASK;
            natask.uSrcIP=str2uint(inet_ntoa(addrSndr.sin_addr));
            natask.uSrcPort=ntohs(addrSndr.sin_port);//send sender's addresss to receiver
            ret=sndNoneblock(udpSvr,(char*)&natask, sizeof(natAsk),addrRcvr);
            assert(ret==sizeof(natAsk));

            //ret=rcvNoneblock(udpSvr,szbuf,1024,addr);
        }
        else if(nMsg==MSGREADY)
        {
            assert(ret== sizeof(natRdy));
            memcpy(&natRdy,szbuf,ret);

            if(addr==addrRcvr)
            {//receiver ready, notify to sender
                ret=sndNoneblock(udpSvr,(char*)&natRdy, sizeof(natRdy),addrSndr);
                assert(ret==sizeof(natRdy));//notify to sender
                prntlog("notify to sender\n");
            }
            else if(addr==addrSndr)
            {//sender ready, notify to receiver
                ret=sndNoneblock(udpSvr,(char*)&natRdy, sizeof(natRdy),addrRcvr);
                assert(ret==sizeof(natRdy));//notify to receiver
                prntlog("notify to receiver\n");
            }
            else
            {
                prntlog("recv ready form %s:%d.\n",inet_ntoa(addr.sin_addr),ntohs(addr.sin_port));
                assert(0);
            }
        }
        else
        {
            assert(0);
        }
    }
}

int main(int argc,char **argv)
{
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;
    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0)
    {
        prnterr("WSAStartup failed with error: %d\n", err);
        return -1;
    }/**/
#endif
    bSvrRun=true;
    std::thread thrdSvr_;

    /*char szbuf[1024];
    uint32  u=0x01020304;
    uint2str(u,szbuf);
    u=str2uint("123.221.255.212");
    uint2str(u,szbuf);prntlog("%s",szbuf);getwchar();*/

    assert(argc==2);
    if(strcmp(argv[1],"svr")==0)
    {
        //thrdSvr_=std::thread([=]{ thrdSvr(); });
        thrdSvr();
    }
    else if(strcmp(argv[1],"rcvr")==0)
    {
        thrdRcvr(0);
    }
    else if(strcmp(argv[1],"sndr")==0)
    {
        thrdSndr(0);
    }

    return 0;
}