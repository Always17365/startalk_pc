//
// Created by cc on 18-11-6.
//

#ifndef STALK_V2_MSGMANAGER_H
#define STALK_V2_MSGMANAGER_H

#include "EventBus/Object.hpp"
#include "Message/UserMessage.h"
#include "EventBus/EventHandler.hpp"
#include "EventBus/HandlerRegistration.hpp"
#include "entity/im_group.h"
#include "Message/GroupMessage.h"
#include <string>
#include <set>
#include <memory>

class CardManager;
class UserCardMsgManager : public Object
{
public:
    UserCardMsgManager();
    ~UserCardMsgManager() override;

public:
    static void getUserCard(std::shared_ptr<st::entity::ImUserSupplement> &,
                            std::shared_ptr<st::entity::ImUserInfo> &);
    static void getUserMedal(const std::string &xmppId,
                             std::set<st::StUserMedal> &medal);
    static void getUserPhoneNo(const std::string &userId, std::string &phoneNo);
    static void setUserSetting(bool isSetting, const std::string &key,
                               const std::string &subKey, const std::string &value);
    static void getGroupCard(std::shared_ptr<st::entity::ImGroupInfo> &);
    static void getGroupMembers(const std::string &groupId);
    static void updateGroupInfo(std::shared_ptr<st::StGroupInfo> groupinfo);
    static void quitGroup(const std::string &groupId);
    static void destroyGroupMsg(const std::string &groupId);
    static void forbiddenWord(const std::string &groupId, bool modStatus);
    static void getforbiddenWordState(const std::string &groupId);
    static void updateMood(const std::string &mood);
    static std::string getSourceImage(const std::string &netPath);
    static void getMedalUser(int medalId, std::vector<st::StMedalUser> &metalUsers);
    static bool modifyUserMedal(int medalId, bool wear);
};

class UserCardMessageListener : public EventHandler<UpdateUserConfigMsg>
//        , public EventHandler<AllFriends>
    , public EventHandler<UpdateGroupMember>
    , public EventHandler<GroupMemberMessage>
    , public EventHandler<IncrementConfig>
    , public EventHandler<ForbiddenWordGroupStateMsg>
    , public EventHandler<GetForbiddenWordResult>
{
public:
    explicit UserCardMessageListener(CardManager *mainPanel);
    ~UserCardMessageListener() override;

protected:
    void onEvent(UpdateUserConfigMsg &e) override;
    //    void onEvent(AllFriends &e) override;
    void onEvent(UpdateGroupMember &e) override;
    void onEvent(GroupMemberMessage &e) override;
    void onEvent(IncrementConfig &e) override;
    void onEvent(ForbiddenWordGroupStateMsg &) override;
    void onEvent(GetForbiddenWordResult &) override;

private:
    CardManager *_pMainPanel;


};
#endif //STALK_V2_MSGMANAGER_H
