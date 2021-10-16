#ifndef _MESSAGEMANAGER_H_
#define _MESSAGEMANAGER_H_

#if _MSC_VER >= 1600
    #pragma execution_character_set("utf-8")
#endif

#include <string>
#include "EventBus/Object.hpp"
#include "EventBus/HandlerRegistration.hpp"
#include "Message/ChatMessage.h"
#include "EventBus/EventHandler.hpp"
#include "Message/GroupMessage.h"
#include "Message/UserMessage.h"
#include "Message/StatusMessage.h"
#include "Message/LoginMessgae.h"
#include "entity/UID.h"
#include "Message/LogicBaseMessage.h"

class ChatMsgManager : public Object
{
public:
    static std::string getNetFilePath(const std::string &localFilePath);
    static std::string getLocalFilePath(const std::string &netFilePath);
    static std::string getSouceImagePath(const std::string &netFilePath);
    static std::string getLocalHeadPath(const std::string &netHeadPath);
    static VectorMessage getUserHistoryMessage(const QInt64 &time,
                                               const QUInt8 &chatType, const st::entity::UID &uid);
    static VectorMessage getNetHistoryMessage(const QInt64 &time, int chatType,
                                              const st::entity::UID &uid, int direction);
    static VectorMessage getUserLocalHistoryMessage(const QInt64 &time,
                                                    const st::entity::UID &uid);
    static VectorMessage getUserFileHistoryMessage(const QInt64 &time,
                                                   const st::entity::UID &uid);
    static VectorMessage getUserImageHistoryMessage(const QInt64 &time,
                                                    const st::entity::UID &uid);
    static VectorMessage getUserLinkHistoryMessage(const QInt64 &time,
                                                   const st::entity::UID &uid);
    static VectorMessage getSearchMessage(const QInt64 &time,
                                          const st::entity::UID &uid, const std::string &text);
    static VectorMessage getAfterMessage(const QInt64 &time,
                                         const st::entity::UID &uid);
    static void getGroupInfo(const std::string &groupId);
    static void sendMessage(S_Message &e);
    static void preSendMessage(const st::entity::ImMessageInfo &message);
    static void sendDownLoadFile(const std::string &strLocalPath,
                                 const std::string &strUri, const std::string &processKey);
    static void sendRevokeMessage(const st::entity::UID &uid,
                                  const std::string &from, const std::string &messageId, const QInt8 &chatType);
    static void setUserSetting(bool isSetting, const std::string &key,
                               const std::string &subKey, const std::string &value);
    static void sendLogReport(const std::string &desc, const std::string &logPath);
    static void setGroupAdmin(const std::string &groupId, const std::string &nick,
                              const std::string &memberJid, bool isAdmin);
    static void removeGroupMember(const std::string &groupId,
                                  const std::string &nick, const std::string &memberJid);
    static void getUserInfo(std::shared_ptr<st::entity::ImUserInfo> &info);
    static std::string uploadFile(const std::string &localFile, bool = false,
                                  const std::string &processKey = std::string());
    static void getRecentSession(std::vector<st::StShareSession> &ss);
    static void getContactsSession(std::vector<st::StShareSession> &ss);
    static void forwardMesssage(const std::string &messsageId,
                                const std::map<std::string, int> &users);
    static void addGroupMember(const std::vector<std::string> &members,
                               const std::string &groupId);
    static void postInterface(const std::string &url, const std::string &params);
    static std::string sendGetRequest(const std::string &url);
    static void updateMessageExtendInfo(const std::string &msgId,
                                        const std::string &info);
    static void sendWebRtcCommand(int msgType, const std::string &json,
                                  const std::string &id);
    static void getUserMedal(const std::string &xmppId,
                             std::set<st::StUserMedal> &medal);
    static void sendSearch(SearchInfoEvent &event);
    static void updateGroupTopic(const std::string &groupId,
                                 const std::string &groupTopic);
    static void getforbiddenWordState(const std::string &groupId);
};

// 消息接收
class ChatViewMainPanel;
class ChatMsgListener : public EventHandler<R_Message>,
    public EventHandler<GroupMemberMessage>
    , public EventHandler<UserCardMessgae>, public EventHandler<R_BlackListMessage>
    , public EventHandler<GroupTopicEvt>, public EventHandler<UpdateOnlineEvent>
    , public EventHandler<FileProcessMessage>,
    public EventHandler<DisconnectToServer>
    , public EventHandler<LoginSuccessMessage>,
    public EventHandler<UpdateGroupMember>
    , public EventHandler<SignalReadState>,
    public EventHandler<RemoveGroupMemberRet>
    , public EventHandler<RevokeMessage>, public EventHandler<UpdateUserConfigMsg>
    , public EventHandler<DestroyGroupRet>, public EventHandler<UpdateGroupInfoRet>
    , public EventHandler<LogReportMessageRet>,
    public EventHandler<GetUsersOnlineSucessEvent>
    , public EventHandler<RecvVideoMessage>,
    public EventHandler<GroupMemberChangeRet>
    , public EventHandler<ChangeHeadRetMessage>, public EventHandler<UpdateMoodRet>
    , public EventHandler<FeedBackLogEvt>
    , public EventHandler<IncrementConfig>
    , public EventHandler<MStateEvt>
    , public EventHandler<WebRtcCommand>
    , public EventHandler<UserMedalChangedEvt>
    , public EventHandler<LoginProcessMessage>
    , public EventHandler<ForbiddenWordGroupStateMsg>
    , public EventHandler<GetForbiddenWordResult>
{
public:
    ChatMsgListener();

    ~ChatMsgListener() override;

public:
    void onEvent(R_Message &e) override;
    void onEvent(GroupMemberMessage &e) override;
    void onEvent(UserCardMessgae &e) override;
    void onEvent(GroupTopicEvt &e) override;
    void onEvent(UpdateOnlineEvent &e) override;
    void onEvent(FileProcessMessage &e) override;
    void onEvent(DisconnectToServer &e) override;
    void onEvent(LoginSuccessMessage &e) override;
    void onEvent(UpdateGroupMember &e) override;
    void onEvent(SignalReadState &e) override;
    void onEvent(MStateEvt &e) override;
    void onEvent(RevokeMessage &e) override;
    void onEvent(UpdateUserConfigMsg &e) override;
    void onEvent(DestroyGroupRet &e) override;
    void onEvent(R_BlackListMessage &e) override;
    void onEvent(UpdateGroupInfoRet &e) override;
    void onEvent(RemoveGroupMemberRet &e) override;
    void onEvent(LogReportMessageRet &e) override;
    void onEvent(GetUsersOnlineSucessEvent &e) override;
    void onEvent(RecvVideoMessage &e) override;
    void onEvent(ChangeHeadRetMessage &e) override;
    void onEvent(GroupMemberChangeRet &e) override;
    void onEvent(UpdateMoodRet &e) override;
    void onEvent(FeedBackLogEvt &e) override;
    void onEvent(IncrementConfig &e) override;
    void onEvent(WebRtcCommand &e) override;
    void onEvent(UserMedalChangedEvt &e) override;
    void onEvent(LoginProcessMessage &e) override;
    void onEvent(ForbiddenWordGroupStateMsg &e) override;
    void onEvent(GetForbiddenWordResult &) override;

private:

};

#endif//
