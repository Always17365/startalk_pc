//
// Created by cc on 2019/09/04.
//

#ifdef _WINDOWS
#include <winsock2.h>
#include<windows.h>
#define ssize_t int
#if defined(WIN32)
#define WIN32_LEAN_AND_MEAN
#endif
#define evutil_socket_t intptr_t
#endif
#if defined(_WINDOWS)
#include <string>
#include <WS2tcpip.h>
#else

#include <cstring>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#endif

struct StHostInfo {
    int ai_family;
    unsigned long time;
    std::string ip;
};

bool takeHostInfo(struct StHostInfo *info, const char *host) ;
