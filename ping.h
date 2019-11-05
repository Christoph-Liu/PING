#pragma once

#include <winsock2.h>

const u_char ICMP_REQUEST = 8;
const u_char ICMP_REPLY = 0;
const u_char ICMP_UNREACHABLE = 11;
const u_char ICMP_UNREACHABLE_INET = 0;
const u_char ICMP_UNREACHABLE_HOST = 1;
const u_char ICMP_UNREACHABLE_PROTO = 2;
const u_char ICMP_UNREACHABLE_PORT = 3;

const int ICMP_DATA_SIZE = 32;
const int IP_MAX_DATA_SIZE = 1500;

const int ICMP_TIMEOUT = 3000;

typedef struct {
    u_char HeaderLength:4;      /*首部长度*/
    u_char version:4;           /*版本号*/
    u_char serviceType;         /*服务类型*/
    u_short totalLength;        /*报文总长度*/
    u_short identifier;         /*标识*/
    u_short FragmentedOffset:13;/*片偏移*/
    u_short flags:3;            /*标志*/
    u_char liveTime;            /*生存时间*/
    u_char protocal;            /*协议*/
    u_short headerChecksum;     /*首部校验和*/
    u_long sourceAddress;       /*源地址*/
    u_long destinationAddress;  /*目的地址*/
}IPHeader;

typedef struct {
    u_char type;                /*类型*/
    u_char code;                /*代码*/
    u_short checksum;           /*icmp报文校验和*/
    u_short identifierId;       /*标识符*/
    u_short sequenceId;         /*序列号*/
}ICMPHeader;

typedef struct {
    in_addr ipAddress;
    int len;
    DWORD time;
    u_char TTL;
    u_short id;
}PareseResult;

class Ping {
    private:
        SOCKET pSocket;
        sockaddr_in pDestAddress;
        char pSendBuffer[sizeof(ICMPHeader) + ICMP_DATA_SIZE];
        char pRecvBuffer[IP_MAX_DATA_SIZE];
    public:
        Ping(char *IPAddress);
        ~Ping();
        u_short caluateChecksum(u_short *buffer, int bufferSize);
        bool parseBuffer(char *buffer, int bufferLen, PareseResult &ret);
        void sendGroup(u_short seqId);
        bool recvGroup(PareseResult &ret);
};