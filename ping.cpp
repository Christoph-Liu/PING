#include <stdio.h>
#include <iostream>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "ping.h"

#pragma comment(lib, "ws2_32.lib")

Ping::Ping(char *IPAddress) {
    int pRet;
    WSAData pWsaData;
    pRet = WSAStartup(MAKEWORD(2, 2), &pWsaData);
    if(pRet != 0) {
        printf("WSAStartup failed...\n");
        printf("code: %d", WSAGetLastError());
        exit(0);
    }
    u_long pIPAddress = inet_addr(IPAddress);
    if(pIPAddress == INADDR_NONE) {
        hostent *pHostent = gethostbyname(IPAddress);
        if(pHostent)
            pIPAddress = (*(in_addr *)(pHostent->h_addr_list[0])).S_un.S_addr;
        else {
            printf("Invalid ip address...\n");
            exit(0);
        }
    }
    pDestAddress.sin_family = AF_INET;
    pDestAddress.sin_addr.S_un.S_addr = pIPAddress;
    pSocket = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if(pSocket == SOCKET_ERROR) {
        printf("create socket failed...\n");
        printf("code: %d", WSAGetLastError());
        exit(0);
    }
    pRet = setsockopt(pSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&ICMP_TIMEOUT, sizeof(ICMP_TIMEOUT));
    if(pRet == SOCKET_ERROR) {
        printf("set sendtime failed...\n");
        printf("code: %d", WSAGetLastError());
        exit(0);
    }
    pRet = setsockopt(pSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&ICMP_TIMEOUT, sizeof(ICMP_TIMEOUT));
    if(pRet == SOCKET_ERROR) {
        printf("set recvtime failed...\n");
        printf("code: %d", WSAGetLastError());
        exit(0);
    }
    printf("PING %s with 32 bytes of data.\n", IPAddress);
}

Ping::~Ping() {
    closesocket(pSocket);
    WSACleanup();
}

u_short Ping::caluateChecksum(u_short *buffer, int bufferSize) {
    u_long checksum = 0;
    while(bufferSize > 1) {
        checksum += *buffer++;
        bufferSize -= sizeof(u_short);
    }
    if(bufferSize)
        checksum += *(u_short *)buffer;
    checksum = (checksum >> 16) + (checksum & 0xffff);
    checksum += (checksum >> 16);
    return (u_short)(~checksum);
}

bool Ping::parseBuffer(char *buffer, int bufferLen, PareseResult &ret) {
    IPHeader *pIpHeader = (IPHeader *)buffer;
    int ipHeaderLen = pIpHeader->HeaderLength * 4;
    ICMPHeader *pIcmpHeader = (ICMPHeader *)(buffer + ipHeaderLen);
    u_short rcvId, rcvSeq;
    if(pIcmpHeader->type == ICMP_REPLY) {
        rcvId = pIcmpHeader->identifierId;
        rcvSeq = pIcmpHeader->sequenceId;
    } else {
        return false;
    }
    if(rcvId != (u_short)GetCurrentProcessId() || rcvSeq != ret.id)
        return false;
    ret.ipAddress.S_un.S_addr = pIpHeader->sourceAddress;
    ret.len = bufferLen - ipHeaderLen - sizeof(ICMPHeader);
    ret.time = GetTickCount() - *(DWORD *)(buffer + ipHeaderLen + sizeof(ICMPHeader));
    ret.TTL = pIpHeader->liveTime;
    return true;
}

void Ping::sendGroup(u_short seqId) {
    memset(pSendBuffer, 0, sizeof(pSendBuffer));
    ICMPHeader *pIcmpHeader = (ICMPHeader *)pSendBuffer;
    pIcmpHeader->type = ICMP_REQUEST;
    pIcmpHeader->code = 0;
    pIcmpHeader->checksum = 0;
    pIcmpHeader->identifierId = (u_short)GetCurrentProcessId();
    pIcmpHeader->sequenceId = seqId;
    DWORD currentTime = GetTickCount();
    memcpy(pSendBuffer+sizeof(ICMPHeader), (char *)&currentTime, sizeof(currentTime));
    pIcmpHeader->checksum = caluateChecksum((u_short *)pSendBuffer, sizeof(ICMPHeader) + ICMP_DATA_SIZE);
    int pRet = sendto(pSocket, pSendBuffer, sizeof(pSendBuffer), 0, (sockaddr *)&pDestAddress, sizeof(pDestAddress));
    if(pRet == SOCKET_ERROR) {
        printf("send group %u failed...\n", (u_long)seqId);
        printf("code: %d", WSAGetLastError());
        exit(0);
    }
}

bool Ping::recvGroup(PareseResult &ret) {
    while(true) {
        sockaddr_in from;
        int fromLen = sizeof(from);
        int pRet = recvfrom(pSocket, pRecvBuffer, IP_MAX_DATA_SIZE, 0, (sockaddr *)&from, &fromLen);
        if(pRet > 0) {
            if(parseBuffer(pRecvBuffer, pRet, ret))
                return true;
        } else if(WSAGetLastError() == WSAETIMEDOUT) {
            return false;
        }
    }
}

