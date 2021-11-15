//
// Created by may on 2018/8/14.
//

#include "ProtobufStack.h"

#include <iostream>
#include "Util/utils.h"
#include "Util/Log.h"
#include "QTalkSecurity.h"
#include "LogicBase.h"
#include "MessageManager.h"
#include "BaseUtil.hpp"
#include "pb2json.h"

namespace st {
namespace Protocol {

ProtobufStack::ProtobufStack(LogicBase *pLogicBase)
    : _pLogicBase(pLogicBase),
      _loginUser("")
{
    _pSocket = new ProtobufStream();
    _pSocket->setStreamDelegate(this);
    auto check_network_fun = [this]() -> bool {
        if (nullptr == _pSocket)
        {
            return false;
        }

        int retry = 3;
        std::string host = _pSocket->_host;

        while (retry > 0)
        {
            StHostInfo info;
            takeHostInfo(&info, host.data());

            if (info.ip.empty() && _pLogicBase) {
                retry--;
            } else {
                return true;
            }
        }

        error_log("no network by {0}", host);
        //
        _pSocket->closeSocket();
        _pLogicBase->onTcpDisconnect("network abnormality", false);
        return false;
    };
    _task = new DelayTask(10000, "-- check network", check_network_fun);
}

ProtobufStack::~ProtobufStack()
{
    delete _task;

    if (nullptr != _pSocket) {
        delete _pSocket;
        _pSocket = nullptr;
    }
}

void ProtobufStack::onMessageReceived(ProtoMessage *message)
{
    _pLogicBase->stopTask();

    if (_task) {
        if (_task->isRuning()) {
            _task->update();
        } else {
            _task->start();
            info_log(" --- start end");
        }
    }

    static bool hasError = false;

    if (message != nullptr && message->IsInitialized()) {
        debug_log("received message, signal type is {0}", message->signaltype());

        switch (message->signaltype()) {
        case SignalTypePresence: {
            PresenceMessage presenceMsg;
            bool isOk = presenceMsg.ParseFromString(message->message());

            if (isOk && _pLogicBase) {
                int key = presenceMsg.definedkey();
                debug_log("presenceMsgKey: {0}", key);
                _pLogicBase->onRecvPresenceMessage(*message, presenceMsg);
            }
        }
        break;

        case SignalTypeIQ: { // IQ消息
            info_log("iq {}", st::toJson(message));

            if (_pLogicBase) {
                IQMessage iqMessage;
                bool isOk = iqMessage.ParseFromString(message->message());

                if (isOk) {
                    IQMessageEvt iqMsgEvt;
                    trancelateIQMsg(iqMessage, &iqMsgEvt);
                    _pLogicBase->onRecvIQMessage(iqMsgEvt);
                }
            }
        }
        break;

        case SignalTypeSucceededResponse: {
            hasError = false;
            flag_user = flag_password = "";
            sendBind();
            break;
        }

        case SignalTypeFailureResponse: {
            if (_pLogicBase) {
                AuthMessage authMessage;
                bool ok = authMessage.ParseFromString(message->message());

                if (ok) {
                    const std::string &error = authMessage.msgid();

                    if ("cancel-rsa" == error) {
                        info_log("cancel-rsa");

                        if (_loginUser.basename() != flag_user || _password != flag_password) {
                            flag_user = _loginUser.basename();
                            flag_password = _password;
                            info_log("write list user, retry login...");
                            sendAuthNoKey();
                            break;
                        }
                    }

                    //
                    _pSocket->closeSocket();
                    _pLogicBase->onConnectedFailed(error);
                    LogicBaseMsgManager::onAuthFailed(error);
                    hasError = true;
                    error_log("socket error : {0}", error);
                }
            }

            break;
        }

        case SignalTypeChat: // 二人聊天消息
        case SignalTypeGroupChat: // 群聊消息
        case SignalTypeConsult:
        case SignalTypeSubscription:
        case SignalTypeNote:
        case SignalTypeHeadline: {
            _pLogicBase->receiveChatMessage(*message);
            break;
        }

        case SignalTypeMState: {
            XmppMessage xmppMsg;

            if (xmppMsg.ParseFromString(message->message())) {
                const std::string &msgId = xmppMsg.messageid();
                QInt64 recvTime = xmppMsg.receivedtime();

                if (nullptr != _pLogicBase) {
                    _pLogicBase->recvOnlineMState(msgId, recvTime);
                }
            }
        }
        break;

        case SignalTypeReadmark: { //消息阅读状态
            if (_pLogicBase) {
                _pLogicBase->recvOnlinesReadMask(*message);
            }
        }
        break;

        // 撤销消息
        case SignalTypeRevoke: {
            XmppMessage xmppMsg;

            if (xmppMsg.ParseFromString(message->message())) {
                if (_pLogicBase && (xmppMsg.messagetype() == MessageTypeRevoke
                                    || xmppMsg.messagetype() == MessageTypeConsultRevoke)) {
                    _pLogicBase->recvRevokeMessage(message->from(), xmppMsg.body(),
                                                   xmppMsg.receivedtime());
                }
            }

            break;
        }

        case SignalTypeWelcome: {
            WelcomeMessage welcomeMsg;
            bool isOk = welcomeMsg.ParseFromString(message->message());
            auto sockmod = welcomeMsg.sockmod();
            std::transform(sockmod.begin(), sockmod.end(), sockmod.begin(), ::toupper);

            if (isOk && supportTLS() && sockmod == "TLS") {
                info_log("start with ssl");
                // 启动Tls
                sendStartTls();
            } else {
                std::string plain;
                plain = st::Security::pwd_key_plain(_loginUser.username(), _password,
                                                    _isNewLogin ? 1 : 2);
                sendAuthWithKey(plain);
            }
        }
        break;

        case SignalProceedTLS: {
            _pSocket->startTLS();
        }
        break;

        case SignalTypeStreamEnd: {
            if (nullptr == _pLogicBase) {
                return;
            }

            StreamEnd streamEnd;

            if (streamEnd.ParseFromString(message->message())) {
                error_log("StreamEnd reason:{0} -> {1}", streamEnd.code(), streamEnd.reason());

                switch (streamEnd.code()) {
                case StreamEndCodeReloginBase:
                    break;

                case StreamEndCodeReloginFromNav: {
                    break;
                }

                default: {
                    // 200 <= code ;1 退到登录界面& 2 不重新登录& 3 提示被踢文案
                    if (streamEnd.code() >= StreamEndCodeNoReloginBase &&
                        streamEnd.code() < StreamEndCodeNoReloginBase + 100) {
                        switch (streamEnd.code() - 200) {
                        case 1:
                            LogicBaseMsgManager::goBackLoginWnd();
                            break;

                        case 2:
                            LogicBaseMsgManager::systemQuit();
                            break;

                        case 3: {
                            const std::string &reason = streamEnd.reason();
                            LogicBaseMsgManager::goBackLoginWnd(reason);
                            break;
                        }

                        default:
                            break;
                        }

                        return;
                    }
                }
                }
            }

            _pLogicBase->onTcpDisconnect("disconnect from server", hasError);
            break;
        }

        case SignalTypeError: {
            XmppMessage xmppMsg;

            if (xmppMsg.ParseFromString(message->message())) {
                if (_pLogicBase) {
                    _pLogicBase->recvErrorMessage(message->from(), xmppMsg);
                }
            }

            break;
        }

        case SignalTypeWebRtc: {
            XmppMessage xmppMsg;

            if (xmppMsg.ParseFromString(message->message())) {
                if (_pLogicBase) {
                    _pLogicBase->recvWebRtc(message->from(), xmppMsg);
                }
            }

            break;
        }

        default:
            break;
        }
    }
}

void ProtobufStack::onSocketConnected()
{
    info_log("socket connected ");
    sendWelcome();
}

void ProtobufStack::onSocketDisConnected()
{
    if (_pLogicBase) {
        _pLogicBase->onTcpDisconnect("disconnect from server", true);
    }
}

static const std::string XmppPbStreamVersion = "1.0";

void ProtobufStack::sendWelcome()
{
    debug_log("send welcome message");
    WelcomeMessage message;
    message.set_user(_loginUser.username());
    message.set_domain(_loginUser.domainname());
    message.set_version(XmppPbStreamVersion);
    ProtoMessage protoMessage;
    protoMessage.set_signaltype(SignalType::SignalTypeWelcome);
    protoMessage.set_message(message.SerializeAsString());
    _pSocket->sendProtobufMessage(&protoMessage);
}

void ProtobufStack::sendStartTls()
{
    StartTLS message;
    ProtoMessage protoMessage;
    protoMessage.set_signaltype(SignalStartTLS);
    protoMessage.set_message(message.SerializeAsString());
    _pSocket->sendProtobufMessage(&protoMessage);
}

/**
* @功能描述 发送优先级别
* @参数
* @date 2018.9.19
*/
void ProtobufStack::sendPriority(const int &level)
{
    PresenceMessage message;
    message.set_key("priority");
    message.set_value(std::to_string(level).data());
    message.set_definedkey(PresenceKeyType::PresenceKeyPriority);
    ProtoMessage package;
    package.set_signaltype(SignalTypePresence);
    package.set_from(_loginUser.fullname());
    package.set_message(message.SerializeAsString());
    _pSocket->sendProtobufMessage(&package);
}

void ProtobufStack::sendAuthWithKey(std::string &key)
{
    AuthMessage message;
    message.set_mechanism("PLAIN");
    message.set_authkey(key);
    ProtoMessage package;
    package.set_signaltype(SignalTypeAuth);
    package.set_from(_loginUser.fullname());
    package.set_message(message.SerializeAsString());
    _pSocket->sendProtobufMessage(&package);
}

void ProtobufStack::sendAuthNoKey()
{
    //
    std::ostringstream plantText;
    plantText << '\0'
              << _loginUser.username()
              << '\0'
              << _password;
    std::string key = utils::base64_encode((unsigned char *)plantText.str().data(),
                                           plantText.str().size());
    //
    AuthMessage message;
    message.set_mechanism("PLAIN");
    message.set_authkey(key);
    ProtoMessage package;
    package.set_signaltype(SignalTypeAuth);
    package.set_from(_loginUser.fullname());
    package.set_message(message.SerializeAsString());
    _pSocket->sendProtobufMessage(&package);
}

bool ProtobufStack::supportTLS()
{
    return AppSetting::instance().with_ssl;
}

void ProtobufStack::setPort(int port)
{
    _pSocket->setPort(port);
}

void ProtobufStack::setHost(const char *host)
{
    _pSocket->setHost(host);
}

bool ProtobufStack::asyncConnect(long seconds)
{
    return _pSocket->asyncConnect(seconds);
}

void ProtobufStack::setPassword(const char *password)
{
    this->_password.assign(password);
}

void ProtobufStack::setNewLogin(bool newLogin)
{
    this->_isNewLogin = newLogin;
}

void ProtobufStack::sendProtobufMessage(const ProtoMessage *message)
{
    if (nullptr != _pSocket) {
        _pSocket->sendProtobufMessage(message);
    }
}

void ProtobufStack::onTlsStarted()
{
    if (_pLogicBase) {
        std::string plain = st::Security::pwd_key_plain(_loginUser.username(),
                                                        _password, _isNewLogin ? 1 : 2);
        sendAuthWithKey(plain);
    }
}

void ProtobufStack::sendIQMessage(IQMessage *message, st::entity::JID *toJid)
{
    ProtoMessage package;
    package.set_signaltype(SignalTypeIQ);
    package.set_from(_loginUser.fullname());

    if (toJid != nullptr) {
        package.set_to(toJid->basename());
    }

    package.set_message(message->SerializeAsString());
    _pSocket->sendProtobufMessage(&package);
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author   cc
  * @date     2018/10/08
  */
st::entity::JID *ProtobufStack::getLoginUser()
{
    return &_loginUser;
}

void ProtobufStack::sendBind()
{
    if (nullptr != _pLogicBase) {
        IQMessage message;
        message.set_key("BIND");

        if (_pLogicBase) {
            message.set_messageid(st::utils::uuid());
        }

        message.set_value(_loginUser.resources());
        sendIQMessage(&message, nullptr);
    }
}


void ProtobufStack::trancelateIQMsg(const IQMessage &message,
                                    IQMessageEvt *iqMsgEvt)
{
    iqMsgEvt->strNamespace = message.namespace_();
    iqMsgEvt->key = message.key();
    iqMsgEvt->value = message.value();
    iqMsgEvt->messageId = message.messageid();
    trancelateHeader(message.header(), &iqMsgEvt->header);
    trancelateBody(message.body(), &iqMsgEvt->body);
    iqMsgEvt->receivedTime = message.receivedtime();
    iqMsgEvt->transferTime = message.transfertime();
    iqMsgEvt->definedKey = message.definedkey();

    //
    for (size_t i = 0; i < message.headers_size(); i++) {
        StStringHeader stHeader;
        trancelateHeader(message.headers(i), &stHeader);
        iqMsgEvt->headers.push_back(stHeader);
    }

    //
    for (size_t j = 0; j < message.bodys_size(); j++) {
        StMessageBody stBody;
        trancelateBody(message.bodys(j), &stBody);
        iqMsgEvt->bodys.push_back(stBody);
    }
}

void ProtobufStack::trancelateHeader(const StringHeader &header,
                                     StStringHeader *stHeader)
{
    for (size_t i = 0; i < header.params_size(); i++) {
        const MessageKeyValue &keyVal = header.params(i);
        stHeader->params[keyVal.key()] = keyVal.value();
    }

    stHeader->key = header.key();
    stHeader->value = header.value();
    stHeader->definedKey = header.definedkey();
}

void ProtobufStack::trancelateBody(const MessageBody &msgBody,
                                   StMessageBody *stBody)
{
    for (size_t i = 0; i < msgBody.headers_size(); i++) {
        StStringHeader stHeader;
        trancelateHeader(msgBody.headers(i), &stHeader);
        stBody->headers.push_back(stHeader);
    }

    stBody->value = msgBody.value();
}

void ProtobufStack::onSocketConnectFailed()
{
    if (_pLogicBase) {
        LogicBaseMsgManager::sendLoginErrMessage("服务器连接失败");
    }
}
}
}
