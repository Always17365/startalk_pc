//
// Created by cc on 2019-01-18.
//

#include "dbPlatForm.h"
#include "LogicManager/LogicManager.h"
#include "Util/nJson/nJson.h"

dbPlatForm &dbPlatForm::instance()
{
    static dbPlatForm platform;
    return platform;
}

std::shared_ptr<st::entity::ImUserInfo> dbPlatForm::getUserInfo(
    const std::string &xmppId, const bool &readDb)
{
    std::string userId = xmppId;
    auto indx = xmppId.find('/');

    if (indx != std::string::npos) {
        userId = userId.substr(0, indx);
    }

    std::shared_ptr<st::entity::ImUserInfo> info = _userInfoMap[xmppId];

    if (info && !readDb) {
        return info;
    } else { // 如果内存中没有则去数据库中查找
        std::shared_ptr<st::entity::ImUserInfo> userinfo =
            LogicManager::instance()->getDatabase()->getUserInfoByXmppId(xmppId);
        _userInfoMap[xmppId] = userinfo;
        return userinfo;
    }
}


std::shared_ptr<st::entity::ImGroupInfo> dbPlatForm::getGroupInfo(
    const std::string &xmppId, const bool &readDb)
{
    std::shared_ptr<st::entity::ImGroupInfo> info = _groupInfoMap[xmppId];

    if (info && !readDb) {
        return info;
    } else { // 如果内存中没有则去数据库中查找
        std::shared_ptr<st::entity::ImGroupInfo> groupinfo =
            LogicManager::instance()->getDatabase()->getGroupInfoByXmppId(xmppId);
        _groupInfoMap[xmppId] = groupinfo;
        return groupinfo;
    }

    return nullptr;
}

/**
 *
 * @param groups
 */
void dbPlatForm::getAllGroup(std::vector<st::entity::ImGroupInfo> &groups)
{
    LogicManager::instance()->getDatabase()->getAllGroup(groups);
}

bool dbPlatForm::isHotLine(const std::string &jid)
{

    if (_hotLines.empty()) {
        std::string strHotLines;
        LogicManager::instance()->getDatabase()->getHotLines(strHotLines);

        if (strHotLines.empty()) {
            return false;
        }

        nJson obj = Json::parse(strHotLines);

        if (nullptr != obj ) {
            bool ret = obj["ret"];

            if (ret) {
                nJson data = Json::get<nJson >(obj, "data");
                auto allHotLines = Json::get<nJson>(data, "allhotlines");

                for (const nJson &item : allHotLines) {
                    _hotLines.insert(Json::convert<std::string>(item, ""));
                }
            }
        }
    }

    return _hotLines.find(jid) != _hotLines.end();
}

std::shared_ptr<std::vector<std::shared_ptr<st::entity::ImSessionInfo> > >
dbPlatForm::reloadSession()
{
    return LogicManager::instance()->getDatabase()->reloadSession();
}

std::vector<st::StUserCard> dbPlatForm::getGroupMemberInfo(
    const std::vector<std::string> &arMembers)
{
    std::vector<st::StUserCard> ret;

    if (!arMembers.empty()) {
        LogicManager::instance()->getDatabase()->getGroupMemberInfo(arMembers, ret);
    }

    return ret;
}

/**
 *
 * @param masks
 */
void dbPlatForm::setMaskNames(const std::map<std::string, std::string> &masks)
{
    std::lock_guard<st::util::spin_mutex> lock(sm);
    mask_names = masks;
}

std::string dbPlatForm::getMaskName(const std::string &xmppId)
{
    std::lock_guard<st::util::spin_mutex> lock(sm);
    auto itFind = mask_names.find(xmppId);

    if (itFind != mask_names.end()) {
        return itFind->second;
    } else {
        return std::string();
    }
}

//
void dbPlatForm::setHotLines(const std::set<std::string> &hotlines)
{
    std::lock_guard<st::util::spin_mutex> lock(sm);
    _hotLines = hotlines;
}

void dbPlatForm::setMedals(const std::vector<st::entity::ImMedalList> &medals)
{
    std::lock_guard<st::util::spin_mutex> lock(sm);
    _medals = medals;
}

st::entity::ImMedalList dbPlatForm::getMedal(const int &id)
{
    std::lock_guard<st::util::spin_mutex> lock(sm);
    auto itFind = std::find_if(_medals.begin(),
    _medals.end(), [id](const st::entity::ImMedalList & medal) {
        return medal.medalId == id;
    });

    if (itFind == _medals.end()) {
        return st::entity::ImMedalList();
    } else {
        return *itFind;
    }
}

void dbPlatForm::getAllMedals(std::vector<st::entity::ImMedalList> &medals)
{
    std::lock_guard<st::util::spin_mutex> lock(sm);
    medals = this->_medals;
}
