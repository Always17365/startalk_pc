#ifndef _CARDMANAGER_H_
#define _CARDMANAGER_H_


#include <QWidget>
#include <QMap>
#include <mutex>
#include <map>
#include <set>
#include <QPointer>
#include "entity/UIEntity.h"
#include "entity/im_userSupplement.h"
#include "entity/im_config.h"
#include "entity/im_group.h"
#include "include/CommonStrcut.h"
#include "Util/Spinlock.h"
#include "entity/im_user.h"

class user_card;

class GroupCard;

class UserCardMsgManager;

class UserCardMessageListener;

class CardManager : public QWidget
{
    Q_OBJECT

public:
    CardManager();

    ~CardManager() override;

public:
    void getPhoneNo(const std::string &userId);

    void updateUserConfig(const std::vector<st::entity::ImConfig> &arConfigs);
    void updateUserConfig(const std::map<std::string, std::string> &deleteData,
                          const std::vector<st::entity::ImConfig> &arImConfig);

    //    void onRecvFriends(const std::vector<st::Entity::IMFriendList> &friends);

    void starUser(const std::string &userId);

    void addBlackList(const std::string &userId);

    void setUserMaskName(const std::string &userId, const std::string &maskName);

    void setUserMood(const std::string &mood);

    void onRecvGroupMember(const std::string &groupId,
                           const std::map<std::string, st::StUserCard> &userCards,
                           const std::map<std::string, QUInt8> &userRole);
    void onForbiddenWordGroupState(const std::string &groupId, bool status);

public:
    void updateGroupInfo(const QString &GroupId, const QString &groupName,
                         const QString &topic, const QString &desc);

    void quitGroup(const QString &groupId);

    void destroyGroup(const QString &groupId);

    std::string getSourceHead(const std::string &headLink);

Q_SIGNALS:

    void showUserCardSignal();

    void showGroupCardSignal();

    void gotPhoneNo(const std::string &, const std::string &);

    void sgOpenNewSession(const StSessionInfo &);

    void sgSwitchCurFun(int);

    void sgUpdateGroupMember(std::map<std::string, st::StUserCard>,
                             std::map<std::string, QUInt8> userRole);

    void sgJumpToStructre(const QString &);

    void sgOperator(const QString &desc);

    void sgShowHeadWnd(const QString &, bool);

    void forbiddenWordSignal(const QString &groupId, bool);

public slots:

    void shwoUserCard(const QString &userId);

    void showGroupCard(const QString &groupId);

    void updateGroupMember(std::map<std::string, st::StUserCard> userCards,
                           std::map<std::string, QUInt8> userRole);

    void forbiddenWordSlot(const QString &groupId, bool);

private:
    void showUserCardSlot();

    void showGroupCardSlot();

private: //data
    QVector<std::string> _arStarContact; // 星标联系人
    QVector<std::string> _arBlackList; // 黑名单
    //    QVector<std::string> _arFriends;   // 好友
    QMap<std::string, std::string> _mapMaskNames; //

private:
    QPointer<user_card> _pUserCard;
    QPointer<GroupCard> _groupCard;

    //    UserCardMsgManager *_pMsgManager;
    UserCardMessageListener *_pMsgListener = nullptr;
    std::shared_ptr<st::entity::ImUserSupplement> _imuserSup;
    std::shared_ptr<st::entity::ImUserInfo>   _userInfo;
    std::shared_ptr<st::entity::ImGroupInfo> _imGroupSup;
    std::set<st::StUserMedal> _user_medal;

private:
    st::util::spin_mutex sm;
};

#endif // _CARDMANAGER_H_
