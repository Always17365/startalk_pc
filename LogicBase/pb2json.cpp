//
// Created by cc on 2020/4/3.
//

#include "pb2json.h"
#include "../QtUtil/nJson/nJson.h"


void dealMessageKeyValue(const MessageKeyValue* keyValue, nJson& obj)
{
    obj["key"] = keyValue->key().data();
    obj["value"] = keyValue->value().data();
}

void dealStringHeader(const StringHeader* header, nJson& objHeader)
{
    objHeader["key"] = header->key().data();
    objHeader["value"] = header->value().data();
    objHeader["StringHeaderType"] = header->definedkey();

    if(header->params_size() > 0)
    {
        nJson params;
        for(const auto& it : header->params())
        {
            nJson objKeyValue;
            dealMessageKeyValue(&it, objKeyValue);
            params.push_back(objKeyValue);
        }
        objHeader["params"] = params;
    }
}

//
void dealMessageBody(const MessageBody* body, nJson& objBody)
{
    if(body->has_value())
        objBody["value"] = body->value().data();

    if(body->headers_size() > 0)
    {
        nJson headers;
        for(const auto& it : body->headers())
        {
            nJson header;
            dealStringHeader(&it, header);
            headers.push_back(header);
        }
        objBody["headers"] = headers;
    }


    if(body->bodys_size() > 0)
    {
        nJson bodys;
        for(const auto& it : body->bodys())
        {
            nJson tmpBody;
            dealMessageBody(&it, tmpBody);
            bodys.push_back(tmpBody);
        }
        objBody["bodys"] = bodys;
    }

//    obj["body"] = objBody;
}


//
void dealAuthMessage(const AuthMessage* msg, nJson& obj)
{
    nJson auth;
    if(msg->has_mechanism())
        auth["mechanism"] = msg->mechanism().data();
    if(msg->has_method())
        auth["method"] = msg->method().data();
    if(msg->has_msgid())
        auth["msgId"] = msg->msgid().data();
    if(msg->has_authkey())
        auth["authKey"] = msg->authkey().data();

    if(msg->has_otherbody())
    {
        nJson objBody;
        dealMessageBody(&msg->otherbody(), objBody);
        auth["otherBody"] = objBody;
    }

    obj["message"] = auth;
}

//
void dealXMessage(const XmppMessage* msg, nJson &obj)
{
    nJson xmpp;
    if(msg->has_messagetype())
        xmpp["messageType"] = msg->messagetype();
    if(msg->has_clienttype())
        xmpp["clientType"] = msg->clienttype();
    if(msg->has_clientversion())
        xmpp["clientVersion"] = msg->clientversion();
    if(msg->has_namespace_())
        xmpp["namespace"] = msg->namespace_().data();
    if(msg->has_key())
        xmpp["key"] = msg->key().data();
    if(msg->has_value())
        xmpp["value"] = msg->value().data();
    if(msg->has_messageid())
        xmpp["messageId"] = msg->messageid().data();
    if(msg->has_receivedtime())
        xmpp["receivedTime"] = msg->receivedtime();
    if(msg->has_transfertime())
        xmpp["transferTime"] = msg->transfertime();

    if(msg->has_header())
    {
        nJson objHeader;
        dealStringHeader(&msg->header(), objHeader);
        xmpp["header"] = objHeader;
    }

    if(msg->has_body())
    {
        nJson objBody;
        dealMessageBody(&msg->body(), objBody);
        xmpp["body"] = objBody;
    }

    if(msg->headers_size() > 0)
    {
        nJson headers;
        for(const auto& it : msg->headers())
        {
            nJson header;
            dealStringHeader(&it, header);
            headers.push_back(header);
        }
        xmpp["headers"] = headers;
    }

    if(msg->bodys_size() > 0)
    {
        nJson bodys;
        for(const auto& it : msg->bodys())
        {
            nJson tmpBody;
            dealMessageBody(&it, tmpBody);
            bodys.push_back(tmpBody);
        }
        xmpp["bodys"] = bodys;
    }

    obj["message"] = xmpp;
}

//
void dealPresenceMessage(const PresenceMessage* msg, nJson& obj)
{
    nJson presence;
    if(msg->has_namespace_())
        presence["namespace"] = msg->namespace_().data();
    if(msg->has_key())
        presence["key"] = msg->key().data();
    if(msg->has_value())
        presence["value"] = msg->value().data();
    if(msg->has_messageid())
        presence["messageId"] = msg->messageid().data();
    if(msg->has_receivedtime())
        presence["receivedTime"] = msg->receivedtime();
    if(msg->has_transfertime())
        presence["transferTime"] = msg->transfertime();
    if(msg->has_definedkey())
        presence["definedKey"] = msg->definedkey();
    if(msg->has_categorytype())
        presence["categoryType"] = msg->categorytype();

    if(msg->has_header())
    {
        nJson objHeader;
        dealStringHeader(&msg->header(), objHeader);
        presence["header"] = objHeader;
    }

    if(msg->has_body())
    {
        nJson objBody;
        dealMessageBody(&msg->body(), objBody);
        presence["body"] = objBody;
    }

    if(msg->headers_size() > 0)
    {
        nJson headers;
        for(const auto& it : msg->headers())
        {
            nJson header;
            dealStringHeader(&it, header);
            headers.push_back(header);
        }
        presence["headers"] = headers;
    }

    if(msg->bodys_size() > 0)
    {
        nJson bodys;
        for(const auto& it : msg->bodys())
        {
            nJson tmpBody;
            dealMessageBody(&it, tmpBody);
            bodys.push_back(tmpBody);
        }
        presence["bodys"] = bodys;
    }

    obj["message"] = presence;
}

//
void dealIQMessage(const IQMessage* msg, nJson& obj)
{
    nJson iq;
    if(msg->has_namespace_())
        iq["namespace"] = msg->namespace_().data();
    if(msg->has_key())
        iq["key"] = msg->key().data();
    if(msg->has_value())
        iq["value"] = msg->value().data();
    if(msg->has_messageid())
        iq["messageId"] = msg->messageid().data();
    if(msg->has_receivedtime())
        iq["receivedTime"] = msg->receivedtime();
    if(msg->has_transfertime())
        iq["transferTime"] = msg->transfertime();
    if(msg->has_definedkey())
        iq["definedKey"] = msg->definedkey();

    if(msg->has_header())
    {
        nJson objHeader;
        dealStringHeader(&msg->header(), objHeader);
        iq["header"] = objHeader;
    }

    if(msg->has_body())
    {
        nJson objBody;
        dealMessageBody(&msg->body(), objBody);
        iq["body"] = objBody;
    }

    if(msg->headers_size() > 0)
    {
        nJson headers;
        for(const auto& it : msg->headers())
        {
            nJson header;
            dealStringHeader(&it, header);
            headers.push_back(header);
        }
        iq["headers"] = headers;
    }

    if(msg->bodys_size() > 0)
    {
        nJson bodys;
        for(const auto& it : msg->bodys())
        {
            nJson tmpBody;
            dealMessageBody(&it, tmpBody);
            bodys.push_back(tmpBody);
        }
        iq["bodys"] = bodys;
    }

    obj["message"] = iq;
}

//
void dealWelcomeMsg(const WelcomeMessage* msg, nJson& obj)
{
    nJson welcome;
    if(msg->has_domain())
        welcome["domain"] = msg->domain().data();
    if(msg->has_version())
        welcome["version"] = msg->version().data();
    if(msg->has_user())
        welcome["user"] = msg->user().data();
    if(msg->has_sockmod())
        welcome["sockmod"] = msg->sockmod().data();
    obj["message"] = welcome;
}

std::string QTalk::toJson(const ProtoMessage *message) {

    nJson objMessage;
    if(message->has_options())
        objMessage["options"] = message->options();
    if(message->has_signaltype())
        objMessage["signaltype"] = message->signaltype();
    if(message->has_from())
        objMessage["from"] = message->from().data();
    if(message->has_to())
        objMessage["to"] = message->to().data();
    if(message->has_realfrom())
        objMessage["realfrom"] = message->realfrom().data();
    if(message->has_realto())
        objMessage["realto"] = message->realto().data();
    if(message->has_originfrom())
        objMessage["originfrom"] = message->originfrom().data();
    if(message->has_originto())
        objMessage["originto"] = message->originto().data();
    if(message->has_origintype())
        objMessage["origintype"] = message->origintype().data();
    if(message->has_sendjid())
        objMessage["sendjid"] = message->sendjid().data();

    switch (message->signaltype())
    {
        case SignalTypePresence:
        {
            PresenceMessage presence;
            bool ok = presence.ParseFromString(message->message());
            if(ok)
                dealPresenceMessage(&presence, objMessage);
            break;
        }
        case SignalTypeIQ:
        {
            IQMessage iq;
            bool ok = iq.ParseFromString(message->message());
            if(ok)
                dealIQMessage(&iq, objMessage);
            break;
        }
        case SignalTypeAuth:
        case SignalTypeSucceededResponse:
        case SignalTypeFailureResponse:
        {
            AuthMessage authMessage;
            bool ok = authMessage.ParseFromString(message->message());
            if(ok)
                dealAuthMessage(&authMessage, objMessage);
            break;
        }
        case SignalTypeChat: // 二人聊天消息
        case SignalTypeGroupChat: // 群聊消息
        case SignalTypeConsult:
        case SignalTypeSubscription:
        case SignalTypeNote:
        case SignalTypeHeadline:
        case SignalTypeTyping:
        case SignalTypeReadmark:
        case SignalTypeRevoke:
        case SignalTypeMState:
        case SignalTypeShareLocation:
        case SignalTypeError:
        {
            XmppMessage xMessage;
            bool ok = xMessage.ParseFromString(message->message());
            if(ok)
                dealXMessage(&xMessage, objMessage);
            break;
        }
        case SignalTypeStreamEnd:
        {
            StreamEnd anEnd;
            bool isOk = anEnd.ParseFromString(message->message());
            if(isOk)
            {
                nJson objEnd;
                objEnd["reason"] = anEnd.reason().data();
                objEnd["code"] = anEnd.code();
                objMessage["message"] = objEnd;
            }
            break;
        }
        case SignalTypeWelcome:
        {
            WelcomeMessage welcome;
            bool isOk = welcome.ParseFromString(message->message());
            if(isOk)
                dealWelcomeMsg(&welcome, objMessage);
            break;
        }
        case SignalTypeStreamBegin:
        {
            StreamBegin begin;
            bool isOk = begin.ParseFromString(message->message());
            if(isOk)
            {
                nJson objBegin;
                objBegin["domain"] = begin.domain().data();
                objBegin["version"] = begin.version().data();
                if(begin.bodys_size() > 0)
                {
                    nJson bodys;
                    for(const auto& it : begin.bodys())
                    {
                        nJson tmpBody;
                        dealMessageBody(&it, tmpBody);
                        bodys.push_back(tmpBody);
                    }
                    objBegin["bodys"] = bodys;
                }
                objMessage["message"] = objBegin;
            }
            break;
        }
        case SignalTypeUserConnect:
        {
            UserConnect uConnect;
            bool isOk = uConnect.ParseFromString(message->message());
            if(isOk)
            {
                nJson objConnt;
                objConnt["domain"] = uConnect.domain().data();
                objConnt["version"] = uConnect.version().data();
                objMessage["message"] = objConnt;
            }
            break;
        }
        case SignalStartTLS:
        case SignalProceedTLS:
        {
            StartTLS tlsMsg;
            bool isOk = tlsMsg.ParseFromString(message->message());
            if(isOk)
                objMessage["message"] = "StartTLS";
            break;
        }
        case SignalTypeIQResponse:
        case SignalTypeNormal:
        case SignalTypeTransfor:
        case SignalTypeHeartBeat:
        case SignalTypeChallenge:
        case SignalTypeWebRtc:
        case SignalTypeCarbon:
        case SignalTypeEncryption:
        case SignalTypeCollection:
        case SignalTypeTrans:
            break;
    }

    std::string data  = objMessage.dump();
    return data;
}