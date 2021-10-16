//
// Created by cc on 18-11-11.
//

#ifndef STALK_V2_MESSAGEMANAGER_H
#define STALK_V2_MESSAGEMANAGER_H


#include "EventBus/Event.hpp"
#include <memory>
#include <vector>
#include <set>
#include "EventBus/EventHandler.hpp"
#include "entity/im_user.h"
#include "EventBus/HandlerRegistration.hpp"
#include "Message/LoginMessgae.h"
#include "Message/GroupMessage.h"
#include "Message/UserMessage.h"

class GroupManagerMsgManager : public Object
 {
public:
    static void getStructure(std::vector<std::shared_ptr<st::entity::ImUserInfo>>& structure);
    static void getGroupMembers(std::map<std::string, std::set<std::string>>& structure);
    static void addGroupMember(const std::vector<std::string>& members, const std::string& groupId);
    static void creatGroup(const std::string& groupId, const std::string& groupName);
    static void getUserInfo(std::shared_ptr<st::entity::ImUserInfo>& info);
    static void getRecentSession(std::vector<st::StShareSession> &ss);

};

class UIGroupManager;
class GroupMsgListener : public EventHandler<CreatGroupRet>, public EventHandler<DestroyGroupRet>
//        , public EventHandler<AllFriends>
        , public EventHandler<AllGroupList>
        , public EventHandler<UpdateUserConfigMsg>
        , public EventHandler<IncrementConfig>
        , public EventHandler<IncrementUser>
{
public:
    explicit GroupMsgListener(UIGroupManager* manager);
    ~GroupMsgListener() override;

public:
    void onEvent(CreatGroupRet& e) override;
    void onEvent(DestroyGroupRet& e) override;
//    void onEvent(AllFriends &e) override;
    void onEvent(AllGroupList& e) override ;
    void onEvent(UpdateUserConfigMsg& e) override ;
    void onEvent(IncrementConfig& e) override ;
    void onEvent(IncrementUser& e) override ;

private:
    UIGroupManager*      _pManager;
};


#endif //STALK_V2_MESSAGEMANAGER_H
