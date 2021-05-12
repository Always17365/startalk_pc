//
// Created by may on 2018/8/14.
//

#ifndef LIBEVENTTEST_PROTOBUFSTACK_H
#define LIBEVENTTEST_PROTOBUFSTACK_H


#ifdef _WINDOWS
#include <winsock2.h>
#include<windows.h>
#endif

#include "ProtobufStreamDelegate.h"
#include "Stream.h"
#include "../QtUtil/Entity/JID.h"
#include "Protobuf/message.pb.h"
#include "IQMessage.h"
#include "../include/DelayTask.hpp"

class LogicBase;
namespace QTalk {
    namespace Protocol {

        class ProtobufStack : public QTalk::Protocol::ProtobufStreamDelegate {
        public:
            explicit ProtobufStack(LogicBase *pLogicBase);
            ~ProtobufStack();

        public:
            void setLoginUser(const Entity::JID& jid) {
                this->_loginUser = jid;
            }

        public:
            void onMessageReceived(ProtoMessage *message) override;
            void onSocketConnected() override;
            void onSocketConnectFailed() override;
            void onSocketDisConnected() override;
            void onTlsStarted() override;

        public:
            bool asyncConnect(long seconds = 3);
            void setHost(const char *host);
            void setPort(int port);
            void setPassword(const char *password);
            void setNewLogin(bool isNewLogin);

        public:
            void sendIQMessage(IQMessage *message, QTalk::Entity::JID *toJid);
            void sendPriority(const int &level);
            void sendProtobufMessage(const ProtoMessage *message);
            QTalk::Entity::JID *getLoginUser();

        private:
            void sendWelcome();
            void sendAuthWithKey(std::string &key);
            void sendAuthNoKey();
            void sendBind();
            bool supportTLS();
            void sendStartTls();

        public:
            void trancelateIQMsg(const IQMessage &message, IQMessageEvt *iqMsgEvt);
            void trancelateHeader(const StringHeader &header, StStringHeader *stHeader);
            void trancelateBody(const MessageBody &msgBody, StMessageBody *stBody);

        private:
            QTalk::Protocol::ProtobufStream *_pSocket;
            LogicBase *_pLogicBase;
            QTalk::Entity::JID _loginUser;
            std::string        _password;
            DelayTask         *_task;
            bool              _isNewLogin{};

        private: // write list user
            std::string flag_user;
            std::string flag_password;
        };
    }
}

#endif //LIBEVENTTEST_PROTOBUFSTACK_H
