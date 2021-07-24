//
// Created by may on 2018/8/27.
//

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <mutex>
#include <atomic>

#include "../include/ThreadPool.h"
#include "../QtUtil/Utils/utils.h"
#include "../QtUtil/Utils/Log.h"
#include "Stream.h"
#include "../QtUtil/lib/ini/ConfigLoader.h"

#if defined(_WINDOWS)
    #define ssize_t int
    #if defined(WIN32)
        #define _WIN32_WINNT 0x0600
    #endif
    #define evutil_socket_t intptr_t

    #include <WinSock2.h>
    #include <WS2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else

    #include <zconf.h>
    #include <cstdlib>
    #include <cstring>
    #include <netdb.h>
    #include <string>
    #include <ostream>
    #include <sstream>
    #include <fcntl.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <sys/signal.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
#endif

namespace QTalk
{
    namespace Socket
    {

        struct hostInfo
        {
            int ai_family{};
            unsigned long time{};
            std::string ip;
        };

        bool takeHostInfo(struct hostInfo *info, const char *host)
        {
            struct addrinfo *answer, hint
            {
            }, *curr;
            memset(&hint, 0, sizeof(hint));
            hint.ai_family = AF_UNSPEC;
            hint.ai_socktype = SOCK_STREAM;
            char ipstr2[128];
            struct sockaddr_in *sockaddr_ipv4;
            struct sockaddr_in6 *sockaddr_ipv6;
            int ret = getaddrinfo(host, nullptr, &hint, &answer);

            if (ret != 0)
                return false;

            for (curr = answer; curr != nullptr; curr = curr->ai_next)
            {
                switch (curr->ai_family)
                {
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

        SSL_CTX *InitCTX()
        {
            SSL_CTX *ctx = nullptr;
            SSL_load_error_strings();
            ERR_load_crypto_strings();
            OpenSSL_add_all_algorithms();
            SSL_library_init();
            ctx = SSL_CTX_new(SSLv23_client_method());

            if (ctx == nullptr)
            {
                ERR_print_errors_fp(stderr);
                error_log("SSL_CTX_new error");
                abort();
            }

            return ctx;
        }

        class nativeSocket : public Stream
        {

        public:
            ~nativeSocket() override
            {
                _bRun = false;
                delete pool;
                delete sendPool;
            }

        public:
            nativeSocket();

        public:
            bool asyncConnect(long theTimeout) override;
            void closeSocket() override;
            void setHost(const char *host) override;
            void setPort(int port) override;
            void startTLS() override;
            void sendMessage(const std::string &buffer) override;
            void setDelegate(SocketDelegate *delegate) override;

            void close_socket();

        private:
            bool connectToServer(long theTimeout);

        private:
            SocketDelegate  *_delegate{nullptr};
            ThreadPool      *pool{nullptr};
            ThreadPool      *sendPool{nullptr};
            SSL_CTX         *ssl_ctx{nullptr};
            SSL             *ssl{nullptr};

            std::atomic_bool _bRun{false};
            std::atomic_bool _bStopByUser{false};
            std::string      _host;
            int              _port{0};
            int              fd{0};
        };

        Stream *buildNewSocket()
        {
            return new nativeSocket();
        }

        void nativeSocket::setDelegate(SocketDelegate *delegate)
        {
            this->_delegate = delegate;
        }
    }
}

QTalk::Socket::nativeSocket::nativeSocket()
{
#ifdef _WINDOWS
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);

    if (::WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        // error
    }

#endif
    pool = new ThreadPool("native socket pool");
    sendPool = new ThreadPool("native socket send pool");
}

bool QTalk::Socket::nativeSocket::connectToServer(long theTimeout)
{
    bool ret = false;
    pool->enqueue([this, theTimeout, &ret]()
    {
        if (fd > 0)
        {
            warn_log("base socket: there is a re connect occoured, fd is {0}", fd);
            return;
        }

        struct hostInfo info;

        bool succeeded = takeHostInfo(&info, _host.c_str());

        info_log("get host info {0} -> {1}", succeeded, info.ip);

        if (succeeded)
        {
            sockaddr *addr = nullptr;
            struct sockaddr_in sockaddr_ipv4 {};
            struct sockaddr_in6 sockaddr_ipv6 {};

            if (info.ai_family == AF_INET)
            {
                memset(&sockaddr_ipv4, 0, sizeof(sockaddr_ipv4));
                sockaddr_ipv4.sin_family = AF_INET;
                sockaddr_ipv4.sin_port = htons(_port);
                inet_pton(AF_INET, info.ip.c_str(), &sockaddr_ipv4.sin_addr);
#ifdef _WINDOWS
                fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
#else
                fd = socket(PF_INET, SOCK_STREAM, 0);
#endif
                addr = (sockaddr *) &sockaddr_ipv4;
                info_log("new socket {0}", fd);
            }
            else if (info.ai_family == AF_INET6)
            {
                memset(&sockaddr_ipv6, 0, sizeof(sockaddr_ipv6));
                sockaddr_ipv6.sin6_family = AF_INET6;
                sockaddr_ipv6.sin6_port = htons(_port);
                inet_pton(AF_INET, info.ip.c_str(), &sockaddr_ipv6.sin6_addr);
#ifdef _WINDOWS
                fd = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
#else
                fd = socket(PF_INET6, SOCK_STREAM, 0);
#endif
                addr = (sockaddr *) &sockaddr_ipv6;
            }

            // SOCK_STREAM（流套接字，使用TCP协议）SOCK_DGRAM（数据报套接字，使用UDP协议） ...
            //
            int keepalive = 1; // 开启keepalive属性
#ifndef _MACOS
            int keepidle = 5; // 如该连接在5秒内没有任何数据往来,则进行探测
#endif
            int keepinterval = 5; // 探测时发包的时间间隔为5 秒
            int keepcount = 3; // 探测尝试的次数.如果第1次探测包就收到响应了,则后2次的不再发.
#ifdef _WINDOWS
            int result = setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char *)&keepalive, sizeof(keepalive));
            result = setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, (const char *)&keepidle, sizeof(keepidle));
            result = setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, (const char *)&keepinterval, sizeof(keepinterval));
            result = setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (const char *)&keepcount, sizeof(keepcount));
            int error = -1;
            unsigned long flag = 1;
            ioctlsocket((SOCKET)fd, FIONBIO, &flag); //设置非阻塞
            int len = sizeof(int);

            if (connect(fd, reinterpret_cast<const sockaddr *>(&sockaddr_ipv4), sizeof(struct sockaddr)) != 0)
            {
                timeval tm{};
                fd_set set;
                tm.tv_sec  = 3;
                tm.tv_usec = 0;
                FD_ZERO(&set);
                FD_SET(fd, &set);

                if( select(fd + 1, nullptr, &set, nullptr, &tm) > 0)
                {
                    getsockopt(fd, SOL_SOCKET, SO_ERROR, (char *)&error, /*(socklen_t *)*/&len);

                    if(error == 0)
                        ret = true;
                }
            }

#else

            if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (const char *) &keepalive, sizeof(keepalive)) != 0)
            {
                error_log(
                    "base socket: setsockopt SO_KEEPALIVE error. fd is {0}, keepalive is {1}, error is {2}",
                    fd,
                    keepalive,
                    strerror(errno));
                close_socket();
                this->_delegate->onSocketConnectFailed();
                return;
            }

#ifndef _MACOS

            if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPIDLE, (const char *) &keepidle, sizeof(keepidle)) != 0)
            {
                error_log(
                    QTalk::utils::format(
                        "base socket: setsockopt TCP_KEEPIDLE error. fd is %d, error is %s",
                        fd,
                        strerror(errno)));
                close_socket();
                this->_delegate->onSocketConnectFailed();
                return;
            }

#endif

            if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPINTVL, (const char *) &keepinterval, sizeof(keepinterval)) != 0)
            {
                error_log(
                    "base socket: setsockopt TCP_KEEPINTVL error. fd is {0}, keepinterval is {1}, error is {2}",
                    fd,
                    keepinterval,
                    strerror(errno));
                close_socket();
                this->_delegate->onSocketConnectFailed();
                return;
            }

            if (setsockopt(fd, IPPROTO_TCP, TCP_KEEPCNT, (const char *) &keepcount, sizeof(keepcount)) != 0)
            {
                error_log(
                    "base socket: setsockopt TCP_KEEPCNT error. fd is {0}, keepcount is {1}, error is {2}",
                    fd,
                    keepcount,
                    strerror(errno));
                close_socket();
                this->_delegate->onSocketConnectFailed();
                return;
            }

            struct timeval timeout {};

            timeout.tv_sec = theTimeout;

            timeout.tv_usec = 0;

            socklen_t len = sizeof(timeout);

            if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (const char *) &timeout, len) != 0)
            {
                error_log(
                    "base socket: setsockopt SO_SNDTIMEO error. fd is {0}, timeout is {1}, error is {2}",
                    fd,
                    theTimeout,
                    strerror(errno));
                close_socket();
                this->_delegate->onSocketConnectFailed();
                return;
            }

            int flag{};
            flag = fcntl(fd, F_GETFL, 0);
            flag |= O_NONBLOCK;
            fcntl(fd, F_SETFL, flag); //设置非阻塞
            auto connected = connect(fd, reinterpret_cast<const sockaddr *>(addr), sizeof(struct sockaddr));

            if (connected != 0)  //if 1
            {
                if (errno != EINPROGRESS)
                    error_log("connect error :%s\n", strerror(errno));
                else
                {
                    struct timeval tm;
                    tm.tv_sec = theTimeout;
                    tm.tv_usec = 0;
                    fd_set set, rset;
                    FD_ZERO(&set);
                    FD_ZERO(&rset);
                    FD_SET(fd, &set);
                    FD_SET(fd, &rset);
                    int res;
                    res = select(fd + 1, &rset, &set, NULL, &tm);

                    if (res < 0)
                        error_log("network error in connect\n");
                    else if (res == 0)
                        error_log("connect time out\n");
                    else if (1 == res)
                    {
                        if (FD_ISSET(fd, &set))
                            ret = true;
                        else
                            error_log("other error when select:%s\n", strerror(errno));
                    }
                }
            }

            struct sockaddr tmpAddr;

            socklen_t tmpLen = sizeof(tmpAddr);

            int ret = getsockname(fd, &tmpAddr, &tmpLen);

            if(ret == 0)
            {
                struct sockaddr_in *sin = (struct sockaddr_in *)(&tmpAddr);
                auto port = sin->sin_port;
                char addr_buffer[INET_ADDRSTRLEN];
                void *tmp = &(sin->sin_addr);

                if (inet_ntop(AF_INET, tmp, addr_buffer, INET_ADDRSTRLEN) == nullptr)
                {
                }

                info_log("socket: {2}, getsockname {0} -> {1}", port, addr_buffer, fd);
            }
            else
                error_log("getsockname error {0} :", strerror(errno));

//            if (connect(fd, reinterpret_cast<const sockaddr *>(addr), sizeof(struct sockaddr)) != 0) {
//
//                error_log(
//                        "base socket: connect error. fd is {0}, error is {1}",
//                        fd,
//                        strerror(errno));
//
//                close_socket();
//                this->_delegate->onSocketConnectFailed();
//                return;
//            }
#endif // _WINDOWS
        }
    }).get();
    return ret;
}

void QTalk::Socket::nativeSocket::closeSocket()
{
    _bRun = false;
    _bStopByUser = true;
}

void QTalk::Socket::nativeSocket::close_socket()
{
    if(ssl)
    {
        SSL_clear(ssl);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        ssl = nullptr;
    }

    if(ssl_ctx)
    {
        SSL_CTX_free(ssl_ctx);
        ssl_ctx = nullptr;
    }

#ifdef _WINDOWS
    closesocket(fd);
#else
    close(fd);
#endif
    fd = 0;
}

bool QTalk::Socket::nativeSocket::asyncConnect(long theTimeout)
{
    if(fd != 0)
        close_socket();

    bool connectRet = connectToServer(theTimeout);

    if(!connectRet)
    {
        close_socket();
        error_log("connectToServer failed");
        return false;
    }

    pool->enqueue([this]()
    {
        _bStopByUser = false;
        _bRun = true;
        info_log("connected to socket, start send and recv message");
        this->_delegate->onSocketConnected();
        long n = 0;
        const int len = 1024;
        char buffer[len] = {0};
        int ssl_init_retry = 5;

        do
        {
            if (this->ssl != nullptr)
            {
                if (SSL_is_init_finished(ssl))
                {
                    if(!_bRun)
                        break;

                    memset(buffer, '\0', sizeof(buffer));

                    while (_bRun && ssl)
                    {
                        n = static_cast<int>(SSL_read(ssl, buffer, len));
                        int errcode = SSL_get_error(ssl, n);

                        if(errcode == SSL_ERROR_NONE)
                        {
                            if(n > 0)
                            {
                                this->_delegate->onDataReceived(buffer, n);
                                break;
                            }
                        }
                        else if (errcode == SSL_ERROR_WANT_READ)
                        {
                            const std::chrono::milliseconds ms(100);
                            std::this_thread::sleep_for(ms);
                        }
                        else
                        {
                            if(errno == 0 || errno == 35)
                            {
                                // TODO
                            }
                            else
                            {
                                error_log(
                                    "base socket: ssl error, fd is: {0}, error is: {2} -> {1}, error code: {3}",
                                    fd,
                                    strerror(errno), errno, errcode);
                                _bRun = false;
                            }

                            break;
                        }
                    }
                }
                else
                {
                    if(ssl_init_retry > 0)
                    {
                        warn_log("ssl not init success. time:{}", ssl_init_retry);
                        const std::chrono::milliseconds ms(500);
                        std::this_thread::sleep_for(ms);
                        ssl_init_retry -= 1;
                        continue;
                    }
                    else
                    {
                        _bRun = false;
                        break;
                    }
                }
            }
            else
            {
                if(!_bRun)
                    break;

                timeval tm{};
                fd_set set;
                tm.tv_sec = 1;
                tm.tv_usec = 0;
                FD_ZERO(&set);
                FD_SET(fd, &set);
                auto res = select(fd + 1, nullptr, &set, nullptr, &tm);

                if (res > 0)
                {
                    if (FD_ISSET(fd, &set))
                    {
                        n = static_cast<ssize_t>(recv(fd, buffer, static_cast<size_t>(len), 0));

                        if (n > 0)
                            this->_delegate->onDataReceived(buffer, n);
                        else
                        {
                            if (errno != 0 && errno != EINTR && errno != EAGAIN)
                            {
                                error_log(
                                    "base socket: socket read failed. fd is {0}, error is {1}, length is {2}",
                                    fd,
                                    errno, n);
                                _bRun = false;
                                break;
                            }
                        }
                    }
                }
                else if(res == 0)
                {
                    // sleep
                    const std::chrono::milliseconds ms(100);
                    std::this_thread::sleep_for(ms);
                }
                else
                {
                    error_log(
                        "base socket: socket select state error. fd is {0}, error is {1}, length is {2}",
                        fd,
                        errno, n);
                    _bRun = false;
                    break;
                }
            }
        }
        while (_bRun);

        if(!_bStopByUser)
            this->_delegate->onSocketDisConnected();

        close_socket();
        info_log("recv pool out. run:{0}, fd:{1}", _bRun, fd);
    });
    return true;
}


void QTalk::Socket::nativeSocket::setHost(const char *host)
{
    this->_host = host;
}

void QTalk::Socket::nativeSocket::setPort(const int port)
{
    this->_port = port;
}

void QTalk::Socket::nativeSocket::startTLS()
{
    try
    {
        SSL_library_init();
        this->ssl_ctx = InitCTX();
        this->ssl = SSL_new(this->ssl_ctx);      /* create new SSL connection state */
        SSL_set_fd(this->ssl, fd);    /* attach the socket descriptor */
        SSL_set_connect_state(ssl);
        bool flag = true;

        while (flag)
        {
            flag = false;

            if(SSL_connect(ssl) == -1)
            {
                int iret = SSL_get_error(ssl, -1);

                if ((iret == SSL_ERROR_WANT_WRITE) || (iret == SSL_ERROR_WANT_READ))
                    flag = true;
                else
                {
                    error_log("start tls error: sslError{}", iret);
                    // ssl init error
                    _bRun = false;
                }
            }
            else
            {
                _delegate->onTlsStarted();
                break;
            }
        }
    }
    catch ( ...)
    {
        error_log("--- startTLS error");
        _bRun = false;
    }
}

void QTalk::Socket::nativeSocket::sendMessage(const std::string &buffer)
{
    sendPool->enqueue([this, buffer]()
    {
        if (_bRun && buffer.length() > 0)
        {
            if (this->ssl && SSL_is_init_finished(this->ssl))
            {
                do
                {
                    int size = SSL_write(this->ssl, buffer.c_str(), static_cast<int>(buffer.length()));

                    if (size != static_cast<int>(buffer.size()))
                    {
                        int sslError = SSL_get_error(ssl, size);

                        if(size == -1 && (sslError == SSL_ERROR_WANT_READ || sslError == SSL_ERROR_WANT_WRITE))
                            continue;

                        error_log("base socket: SSL error: size:{0} - buffer.length():{1}. error:{2} -> {3}, sslError->{4}",
                                  size,
                                  buffer.length(), strerror(errno), errno, sslError);
                        _bRun = false;
                    }
                    else
                    {
                        // success to sent message
                        break;
                    }
                }
                while (_bRun);
            }
            else
            {
                ssize_t size = send(fd, buffer.c_str(), buffer.length(), 0);

                if (size != static_cast<int>(buffer.size()))
                {
                    error_log("base socket: TCP: your writen is not complated. size:{0} - buffer.length():{1}",
                              size,
                              buffer.length());
                    _bRun = false;
                }
            }
        }
    });
}
