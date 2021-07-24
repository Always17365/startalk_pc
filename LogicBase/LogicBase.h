//
// Created by cc on 2019-08-13.
//
#if _MSC_VER >= 1600
    #pragma execution_character_set("utf-8")
#endif
#ifndef QTALK_V2_LOGICBASE_H
#define QTALK_V2_LOGICBASE_H

#include "../interface/logic/ILogicBase.h"
#include "ProtobufStack.h"
#include "../Platform/Platform.h"
#include "../include/STLazyQueue.h"
#include "../entity/UID.h"

/**
* @description: LogicBase
* @author: cc
* @create: 2019-08-13 18:14
**/
class LogicBaseMsgManager;
class LogicBaseMsgListener;
class LogicBase : public ILogicBase
{
public:
    LogicBase();
    ~LogicBase() override;

public:
    bool tryConnectToServer(const std::string &userName, const std::string &password, const std::string &host, int port, bool isNewLogin) override ;
    std::string chatRsaEncrypt(const std::string &value) override ;
    std::string normalRsaEncrypt(const std::string &value) override ;
    // 销毁群
    void destroyGroup(const std::string &groupId) override;
    // 退出群
    void quitGroup(const std::string &groupId) override;
    // 移除群成员
    void removeGroupMember(const std::string &groupId,
                           const std::string &nickName,
                           const std::string &memberJid) override;
    // 设置群管理/取消群管理
    void setGroupAdmin(const std::string &groupId, const std::string &nickName,
                       const std::string &memberJid, bool isAdmin) override ;
    // 获取好友列表
//    void getFriendList() override ;
    //
    void getGroupMemberById(const std::string &groupId) override;
    //
    void inviteGroupMembers(std::vector<std::string> &users, const std::string &groupId) override ;
    //
    void createGroup(const std::string &groupId) override ;
    //
    void sendHeartbeat() override ;
    //
    bool sendReportMessage(const std::string &msg, std::string &error) override ;
    //
    bool sendReportMail(const std::vector<std::string> &tos,
                        const std::string &strSub,
                        const std::string &body,
                        bool isHtml, std::string &error) override ;

public:
    void startAutoReconnectToServer(const std::string &host = "") override;

public:
    //
    void switchUserStatus(const std::string &status);
    //
    void dealReadedMessage(const std::string &messageId, const std::string &userId, QUInt8 chatType);

public:
    void onTcpDisconnect(const std::string &error, bool hadError = false);
    // 收到错误消息
    void recvErrorMessage(const std::string &fromId, const XmppMessage &xmppMsg);
    // 服务器连接失败
    void onConnectedFailed(const std::string &reason);
    // IQ消息
    void onRecvIQMessage(const IQMessageEvt &evt);
    //
    void onRecvPresenceMessage(const ProtoMessage &message, const PresenceMessage &presenceMsg);
    // 聊天消息
    void receiveChatMessage(ProtoMessage &message);
    // 服务器已接收消息
    void recvOnlineMState(const std::string &messageid, const QInt64 &serverTime);
    // 阅读状态消息
    void recvOnlinesReadMask(const ProtoMessage message);
    // 撤销消息
    void recvRevokeMessage(const std::string &peerId, const MessageBody xmppBody, QInt64 time);
    //
    void forwardMessage(const std::string &messageId, const std::map<std::string, int> &users);

    // 发送撤销消息
    void sendRovokeMessage(const QTalk::Entity::UID &peerId, const std::string &msgFrom, const std::string &messageId, const QInt8 &chatType);
    //
    void sendMessage(const QTalk::Entity::ImMessageInfo &message);

    void parseSendMessageIntoDb(const QTalk::Entity::ImMessageInfo &message, const int messageType);

    //
    void recvWebRtc(const std::string &peerId, const XmppMessage &extendInfo);
    void sendWebRtcCommand(int msgType, const std::string &peerId, const std::string &cmd);

private:
    void parseReceiveMessageIntoDb(const ProtoMessage &protoMsg, const XmppMessage &xmppMsg);
    //
    void dealBindMsg(const IQMessageEvt &evt);
    // 单聊消息已读状态设置
    void sendSignalReadMask(const std::vector<std::string> &ids, const std::string &mask, const std::string &messageTo);
    void
    sendGroupReadMask(const std::map<std::string, QInt64> &ids, const std::string &mask, const std::string &messageTo);

public:
    void stopTask();

public:
    bool _isConnected;

private:
    QTalk::util::spin_mutex sm;

private:
    QTalk::Protocol::ProtobufStack *_pStack{};

    LogicBaseMsgListener *_pMsgListener;

private:
    QTalk::util::spin_mutex _iq_sm;
    std::map<std::string, std::string> _mapIQMessageId; // <id, val>

private:
//    STLazyQueue<QTalk::Entity::ImSessionInfo> *sessionInfoProcedure;
//    STLazyQueue<QTalk::Entity::ImMessageInfo> *msgInfoProcedure;

private:
    DelayTask         *_delayTask {nullptr};
    bool              _autoReconnectToServer {};
    std::string       _host{};
};

#endif //QTALK_V2_LOGICBASE_H
