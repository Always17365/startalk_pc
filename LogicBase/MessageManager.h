//
// Created by cc on 2019-08-13.
//

#ifndef STALK_V2_MESSAGEMANAGER_H
#define STALK_V2_MESSAGEMANAGER_H


/**
* @description: MessageManager
* @author: cc
* @create: 2019-08-13 19:44
**/
#ifndef _MESSAGEMANAGER_H
#define _MESSAGEMANAGER_H

#include <string>
#include "EventBus/Object.hpp"
#include "EventBus/EventHandler.hpp"
#include "EventBus/HandlerRegistration.hpp"
#include "Message/LoginMessgae.h"
#include "IQMessage.h"
#include "Message/ChatMessage.h"
#include "entity/IM_Session.h"
#include "Message/UserMessage.h"
#include "include/CommonStrcut.h"
#include "Message/GroupMessage.h"
#include "Message/StatusMessage.h"
//#include "entity/im_friend_list.h"
#include "entity/im_group.h"
#include "Message/UselessMessage.h"
#include "Util/Entity/JID.h"
#include "Message/LogicBaseMessage.h"

class LogicBaseMsgManager : public Object
{
public:
    // 登录状态信息
    static void sendLoginProcessMessage(const std::string &message);
    // 登录失败消息
    static void sendLoginErrMessage(const std::string &message);
    // 验证失败消息
    static void onAuthFailed(const std::string &message);
    // 断链消息
    static void onDisconnectToServer();
    // 发送的消息被服务器拒绝
    static void recvBlackListMessage(const std::string &messageFrom,
                                     const std::string &messageId);
    // 发送 http 请求
    static void sendHttpReq(const st::HttpRequest &request,
                            const std::function<void(int, const std::string &)> &callback);

public:
    static void onRecvGroupMembers(const std::string &groupId,
                                   std::map<std::string, QUInt8> mapUserRole);
    //
    static void creatGroupResult(const std::string &groupId, bool ret);
    //
    static void onInviteGroupMembers(const std::string &groupId);
    //
    static void onDealBind();
    //
    static void onUpdateTimeStamp();
    //
    static void LoginSuccess(const std::string &strSessionId);
    static void synSeverData();
    //
    static void onUpdateGroupInfo(std::shared_ptr<st::StGroupInfo> info);
    //
    static void onSwitchUserStatus(const std::string &status);
    //
    static void onUserConfigChanged();
    //
    static void onUserJoinGroup(const std::string &groupId,
                                const std::string &memberId, int affiliation);
    //
    static void onStaffChanged();
    //
    static void onFeedLog(const std::string &text);
    //
    static void onDestroyGroup(const std::string &groupId, bool isDestroy);
    //
    static void onRemoveGroupMember(const std::string &groupId,
                                    const std::string &memberId);
    //
    static void updateRevokeMessage(const st::entity::UID &uid,
                                    const std::string &fromId, const std::string &messageId, const QInt64 &time);
    //
    static void updateSignalChatReadState(const std::string &userId,
                                          const std::string &realJid, const std::map<std::string, QInt32> &readMasks);
    //
    static void updateGroupChatReadState(const std::string &groupId,
                                         const std::map<std::string, int > &readedCount);
    //
    static void onRecvVideo(const std::string &peerId);
    //
    static void onRecvChatMessage(R_Message &e);
    //
    static void updateMState(const std::string &userId, const std::string &realJid,
                             const std::string &messageId, const QInt64 &time);

    static void onRecvWebRtcCommand(int msgType, const std::string &jid,
                                    const std::string &command, bool isCarbon);
    //
    static void onMedalListChanged();
    //
    static void onUserMedalChanged();

    static bool retryConnecToServer();

public:
    static void refreshNav();
    static void goBackLoginWnd(const std::string &reson = "");
    static void systemQuit();

    static void forbiddenWordGroupState(const std::string &groupId,
                                        bool status);
};

//
class LogicBase;
class LogicBaseMsgListener : public EventHandler<S_RevokeMessage>
    , public EventHandler<SwitchUserStatusEvt>
    , public EventHandler<ReadedMessage>
    , public EventHandler<HeartBeat>
    , public EventHandler<ForwardMessage>
    , public EventHandler<S_Message>
    , public EventHandler<PreSendMessageEvt>
    , public EventHandler<SWebRtcCommand>
    , public EventHandler<ForbiddenWordGroupMsg>
{
public:
    explicit LogicBaseMsgListener(LogicBase *pLogicBase);
    ~LogicBaseMsgListener() override;

public:
    void onEvent(S_RevokeMessage &e ) override;
    void onEvent(SwitchUserStatusEvt &e ) override;
    void onEvent(ReadedMessage &e ) override;
    void onEvent(HeartBeat &e ) override;
    void onEvent(ForwardMessage &e ) override;
    void onEvent(S_Message &e ) override;
    void onEvent(PreSendMessageEvt &e ) override;
    void onEvent(SWebRtcCommand &e ) override;
    void onEvent(ForbiddenWordGroupMsg &e)override;

private:
    LogicBase            *_pLogicBase;

};

#endif


#endif //STALK_V2_MESSAGEMANAGER_H
