#include <iostream>
#include <string.h>
#include <algorithm>
#include <windows.h>
#include "ping.h"
#include "ping.cpp"

const int maxPackets = 10;
char IPAddress[256];

int main(int argc, char *argv[]) {
    if(argc > 1) 
        strcpy(IPAddress, argv[1]);
    else
        strcpy(IPAddress, "www.baidu.com");
    Ping ping = Ping(IPAddress);
    PareseResult ret;
    int success = 0, totalTime = 0, minTime = 1e9, maxTime = -1;
    for(u_short seq = 1; seq <= maxPackets; ++seq) {
        ping.sendGroup(seq);
        ret.id = seq;
        if(ping.recvGroup(ret)) {
            success++;
            printf("%u bytes from %s (%s): icmp_seq=%u ttl=%u time=%d ms\n", ret.len, IPAddress, 
                inet_ntoa(ret.ipAddress), seq, (u_short)ret.TTL, ret.time);
            totalTime += ret.time;
            minTime = std::min(minTime, (int)ret.time);
            maxTime = std::max(maxTime, (int)ret.time);
        } else {
            printf("timeout...\n");
        }
        Sleep(1000);
    }
    printf("--- %S ping statistics ---\n", IPAddress);
    printf("%d packets transmitted, %d received, %.2f%% packet loss, time %d ms\n", maxPackets, success, (maxPackets - success) * 100 / (maxPackets * 1.0), totalTime);
    printf("rtt min/avg/max = %d/%.2f/%d/ ms\n", minTime, totalTime / (maxPackets * 1.0), maxTime);
    return 0;
}