//
// Created by may on 2018/7/27.
//

#ifndef QTALK_QTALKSOCKET_H
#define QTALK_QTALKSOCKET_H


#include <string>

namespace QTalk {
    namespace Socket {
        class SocketDelegate;

        class Stream {
        public:
            virtual ~Stream() = default;
            Stream() = default;
            Stream(Stream const &) = delete;
            Stream &operator=(Stream const &) = delete;
            virtual void setHost(const char *host) = 0;
            virtual void setPort(int port) = 0;
            virtual bool asyncConnect(long timeout) = 0;
            virtual void closeSocket() = 0;
            virtual void startTLS() = 0;
            virtual void setDelegate(SocketDelegate *delegate) = 0;
            virtual void sendMessage(const std::string &str) = 0;
        };

        //
        Stream *buildNewSocket();

        //
        class SocketDelegate {
        public:
            virtual void onDataReceived(const char *msg, size_t len) = 0;
            virtual void onSocketConnected() = 0;
            virtual void onSocketConnectFailed() = 0;
            virtual void onSocketDisConnected() = 0;
            virtual void onTlsStarted() = 0;
        };
    }
}

#endif //QTALK_QTALKSOCKET_H
