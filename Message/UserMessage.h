﻿#ifndef _USER_MESSAGE_H_
#define _USER_MESSAGE_H_

#include <utility>
#include <vector>
#include <map>
#include <string>
#include <set>
#include "include/CommonStrcut.h"
#include "EventBus/Event.hpp"
#include "entity/im_user.h"
#include "entity/im_userSupplement.h"
#include "entity/im_config.h"
//#include "entity/im_friend_list.h"
#include "entity/im_transfer.h"
#include "entity/UID.h"
#include "entity/im_qr_group.h"
#include "entity/im_qr_content.h"
#include "entity/im_user_status_medal.h"

// 获取用户列表
class GetUserCardMessage : public Event
{
public:
    std::map<std::string, std::map<std::string, int> > mapUserIds; //
};

// 用户列表结果
class UserCardMessgae : public Event
{
public:
    std::vector<st::StUserCard> userCards;
};

// 用户列表结果
class UserCardSupple: public Event
{
public:
    UserCardSupple(std::shared_ptr<st::entity::ImUserSupplement> &imUserSup,
                   std::shared_ptr<st::entity::ImUserInfo> &userInfo)
        : imUserSup(imUserSup), userInfo(userInfo)
    {
    }

public:
    std::shared_ptr<st::entity::ImUserSupplement> &imUserSup;
    std::shared_ptr<st::entity::ImUserInfo> &userInfo;
};

//
class UserPhoneNo : public Event
{
public:
    std::string userId;
    std::string phoneNo;
};

// user setting
class UserSettingMsg : public Event
{
public:
    enum {EM_OPERATOR_INVALID, EM_OPERATOR_SET, EM_OPERATOR_CANCEL};
    //
    QUInt8 operatorType = EM_OPERATOR_INVALID;
    std::string key;
    std::string subKey;
    std::string value;
};

//
class UpdateUserConfigMsg : public Event
{
public:
    std::vector<st::entity::ImConfig> arConfigs;
};

// 增量配置修改
class IncrementConfig : public Event
{
public:
    std::map<std::string, std::string> deleteData;
    std::vector<st::entity::ImConfig> arImConfig;
};

//class AllFriends : public Event
//{
//public:
//    std::vector<st::Entity::IMFriendList> friends;
//};

class UserCardInfo : public Event
{
public:
    explicit UserCardInfo(std::shared_ptr<st::entity::ImUserInfo> &info) : info(
            info) {};

public:
    std::shared_ptr<st::entity::ImUserInfo> &info;
};

class UpdateMoodEvt : public Event
{
public:
    explicit UpdateMoodEvt(std::string mood) : mood(std::move(mood)) {}

public:
    std::string mood;
};

class UpdateMoodRet : public Event
{
public:
    explicit UpdateMoodRet(std::string userId, std::string mood)
        : mood(std::move(mood)), userId(std::move(userId)) {}

public:
    std::string mood;
    std::string userId;
};

class SwitchUserStatusEvt : public Event
{
public:
    explicit SwitchUserStatusEvt(std::string status) : user_status(std::move(
                    status)) {};

public:
    std::string user_status;
};

class SwitchUserStatusRet : public Event
{
public:
    explicit SwitchUserStatusRet(std::string status) : user_status(std::move(
                    status)) {};

public:
    std::string user_status;
};

class FeedBackLogEvt : public Event
{
public:
    explicit FeedBackLogEvt(std::string text) : text(std::move(text)) {};

public:
    std::string text;
};

class IncrementUser : public Event
{
public:
    std::vector<st::entity::ImUserInfo> arUserInfo;
    std::vector<std::string> arDeletes;
};

class UserMedalEvt: public Event
{
public:
    UserMedalEvt(std::string xmppId, std::set<st::StUserMedal> &medal)
        : medal(medal), xmppId(std::move(xmppId))
    {
    }

public:
    std::set<st::StUserMedal> &medal;
    std::string xmppId;
};

class UserMedalChangedEvt : public Event
{
public:
    std::vector<st::entity::ImUserStatusMedal> userMedals;
};

class GetMedalUserEvt : public Event
{
public:
    int medalId;
    std::vector<st::StMedalUser> metalUsers;
};

class ModifyUserMedalStatusEvt : public Event
{
public:
    int medalId = -1;
    bool isWear = false;

    bool result = false;
};
#endif//_USER_MESSAGE_H_
