//
// Created by cc on 2019-01-18.
//

#ifndef STALK_V2_DBPLATFORM_H
#define STALK_V2_DBPLATFORM_H

#include <string>
#include <map>
#include <vector>
#include <set>
#include <memory>

#include "entity/im_user.h"
#include "entity/im_group.h"
#include "entity/IM_Session.h"
#include "entity/im_medal_list.h"
#include "platform_global.h"
#include "include/CommonStrcut.h"
#include "Util/Spinlock.h"

#define DB_PLAT dbPlatForm::instance()

/**
* @description: ${description}
* @author: cc
* @create: 2019-01-18 16:15
**/
class PLATFORMSHARED_EXPORT dbPlatForm
{

public:
    dbPlatForm() = default;

public:
    bool isHotLine(const std::string &jid);
    void setHotLines(const std::set<std::string> &hotlines);

public:
    void setMaskNames(const std::map<std::string, std::string> &masks);
    std::string getMaskName(const std::string &xmppId);
    void getAllGroup(std::vector<st::entity::ImGroupInfo> &groups);

    void setMedals(const std::vector<st::entity::ImMedalList> &medals);
    st::entity::ImMedalList getMedal(const int &id);
    void getAllMedals(std::vector<st::entity::ImMedalList> &medals);

public:
    static dbPlatForm &instance();

public:
    std::shared_ptr<st::entity::ImUserInfo> getUserInfo(const std::string &xmppId,
                                                        const bool &readDb = false);
    std::shared_ptr<st::entity::ImGroupInfo> getGroupInfo(const std::string &xmppId,
                                                          const bool &readDb = false);
    std::shared_ptr<std::vector<std::shared_ptr<st::entity::ImSessionInfo>>>
    reloadSession();
    std::vector<st::StUserCard> getGroupMemberInfo(const std::vector<std::string>
                                                   &arMembers);

private:
    std::map<std::string, std::shared_ptr<st::entity::ImUserInfo> > _userInfoMap;
    std::map<std::string, std::shared_ptr<st::entity::ImGroupInfo> > _groupInfoMap;
    std::set<std::string> _hotLines;

private:
    std::vector<st::entity::ImMedalList> _medals;

private:
    st::util::spin_mutex sm;
    std::map<std::string, std::string> mask_names;
};


#endif //STALK_V2_DBPLATFORM_H
