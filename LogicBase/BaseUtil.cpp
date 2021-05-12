//
// Created by cc on 2020/2/19.
//

#include "BaseUtil.hpp"

bool takeHostInfo(struct StHostInfo *info, const char *host)
{
    {
        struct addrinfo *answer, hint{}, *curr;
        memset(&hint, 0, sizeof(hint));
        hint.ai_family = AF_UNSPEC;
        hint.ai_socktype = SOCK_STREAM;
        char ipstr2[128];
        struct sockaddr_in *sockaddr_ipv4;
        struct sockaddr_in6 *sockaddr_ipv6;

        int ret = getaddrinfo(host, nullptr, &hint, &answer);
        if (ret != 0) {
            return false;
        }

        for (curr = answer; curr != nullptr; curr = curr->ai_next) {
            switch (curr->ai_family) {
                case AF_UNSPEC:
                    //do something here
                    break;
                case AF_INET:
                    info->ai_family = curr->ai_family;
                    sockaddr_ipv4 = reinterpret_cast<struct sockaddr_in *>( curr->ai_addr);
#ifdef _WINDOWS
                    info->ip = inet_ntop(AF_INET, (PVOID)(&sockaddr_ipv4->sin_addr), ipstr2, sizeof(ipstr2));
#else
                    info->ip = inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, ipstr2, sizeof(ipstr2));
#endif
                    break;
                case AF_INET6:
                    info->ai_family = curr->ai_family;
                    sockaddr_ipv6 = reinterpret_cast<struct sockaddr_in6 *>( curr->ai_addr);
#ifdef _WINDOWS
                    info->ip = inet_ntop(AF_INET6, (PVOID)(&sockaddr_ipv6->sin6_addr), ipstr2, sizeof(ipstr2));
#else
                    info->ip = inet_ntop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstr2, sizeof(ipstr2));
#endif
                    break;
                default:
                    break;
            }
        }
        freeaddrinfo(answer);

        return true;
    }
}