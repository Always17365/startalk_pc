//
// Created by may on 2018/8/13.
//

#ifndef LIBEVENTTEST_PROTOBUFSTREAM_H
#define LIBEVENTTEST_PROTOBUFSTREAM_H


#include "Stream.h"
#include <thread>
#include "MemoryStream.h"
#include "ProtobufParser.h"

namespace st {
    namespace Protocol {
        class ProtobufStreamDelegate;
        class Stream;

        class ProtobufStream : public st::Socket::SocketDelegate {
        public:
            explicit ProtobufStream();
            ~ProtobufStream();

        public:
            inline bool asyncConnect(long seconds = 3) {
                return _pSocket->asyncConnect(seconds);
            }

            void setHost(const char *host) {
                _pSocket->setHost(host);
                this->_host = host;
            }

            inline void setPort(const int port) {
                _pSocket->setPort(port);
            }

            void startTLS() {
                _pSocket->startTLS();
            }

            void closeSocket()
            {
                _pSocket->closeSocket();
            }

        public:
            void sendProtobufMessage(const ProtoMessage *message);

        public:
            void onDataReceived(const char *msg, size_t len) override;
            void onSocketConnected() override;
            void onSocketConnectFailed() override;
            void onSocketDisConnected() override;
            void onTlsStarted() override;

        public:
            void setStreamDelegate(ProtobufStreamDelegate *delegate);

        public:
            std::string _host;

        private:
            st::Socket::Stream *_pSocket;
            st::Protocol::ProtobufParser _parser;
            st::Protocol::ProtobufStreamDelegate *_delegate{};
        };

        class ProtobufStreamDelegate {
        public:
            virtual void onMessageReceived(ProtoMessage *message) = 0;
            virtual void onSocketConnected() = 0;
            virtual void onSocketConnectFailed() = 0;
            virtual void onSocketDisConnected() = 0;
            virtual void onTlsStarted() = 0;
        };
    };
}

#endif //LIBEVENTTEST_PROTOBUFSTREAM_H
