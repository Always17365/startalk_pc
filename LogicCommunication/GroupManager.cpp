#include "GroupManager.h"
#include <iostream>
#include <sstream>
#include "DataCenter/NavigationManager.h"
#include "DataCenter/Platform.h"
#include "interface/logic/IDatabasePlug.h"
#include "Util/nJson/nJson.h"
#include "Util/utils.h"
#include "MessageManager.h"
#include "Communication.h"
#include "Util/Log.h"
#include "UserConfig.h"

using namespace st;
using std::string;

GroupManager::GroupManager(Communication *pComm)
    : _pComm(pComm)
{
}

/**
  * @函数名   getUserGroupInfo
  * @功能描述 拉取群资料
  * @参数
     bool
  * @author   cc
  * @date     2018/09/30
  */
bool GroupManager::getUserGroupInfo(MapGroupCard &mapGroups)
{
    long long originVersion = 0;
    LogicManager::instance()->getDatabase()->getGroupMainVersion(originVersion);
    std::ostringstream url;
    url << NavigationManager::instance().getHttpHost()
        << "/muc/get_increment_mucs.qunar"
        << "?v=" << DC.getClientVersion()
        << "&p=" << DC.getPlatformStr()
        << "&u=" << DC.getSelfUserId()
        << "&k=" << DC.getServerAuthKey()
        << "&d=" << DC.getSelfDomain();
    nJson obj;
    obj["u"] = DC.getSelfUserId();
    obj["d"] = DC.getSelfDomain();
    obj["t"] = originVersion;
    string postData = obj.dump();
    string strUrl = url.str();
    std::vector<entity::ImGroupInfo> groups;
    std::vector<string> deleteGroups;
    bool restSts = false;
    long long mainVersion = 0;
    auto callback = [strUrl, &mapGroups, &restSts, &groups, &deleteGroups,
                             &mainVersion](int code,
    const string & responseData) {
        info_log("{0} \n{1}", strUrl, responseData);

        if (code == 200) {
            nJson data = Json::parse(responseData);

            if (data == nullptr) {
                error_log("json paring error");
                return;
            }

            bool ret = Json::get<bool>(data, "ret");

            if (ret) {
                string vs = Json::get<string>(data, "version");
                mainVersion = std::stoll(vs);
                nJson msgList = Json::get<nJson>(data, "data");

                for (auto &item : msgList) {
                    entity::ImGroupInfo group;
                    string groupid = Json::get<string>(item, "M");
                    string localdomain = Json::get<string>(item, "D");
                    group.GroupId = groupid + "@" + localdomain;
                    int flag = Json::get<int>(item, "F");

                    if (flag) {
                        group.LastUpdateTime = 0;
                        mapGroups[localdomain].push_back(group);
                        groups.push_back(group);
                    } else {
                        deleteGroups.push_back(group.GroupId);
                    }
                }

                restSts = true;
                return;
            }
        } else {
            warn_log("请求失败  url: {0}", strUrl);
        }
    };

    if (_pComm) {
        HttpRequest req(strUrl, RequestMethod::POST);
        req.header["Content-Type"] = "application/json;";
        req.body = postData;
        _pComm->addHttpRequest(req, callback);

        if (restSts && !groups.empty()) {
            restSts = LogicManager::instance()->getDatabase()->bulkInsertGroupInfo(groups);
        }

        if (restSts && !deleteGroups.empty()) {
            restSts = LogicManager::instance()->getDatabase()->bulkDeleteGroup(
                          deleteGroups);
            // all top
            std::map<string, string> allTop;
            LogicManager::instance()->getDatabase()->getConfig("kStickJidDic", allTop);

            // 删除置顶群
            for (const auto &groupId : deleteGroups) {
                string uid = st::entity::UID(groupId).toStdString();

                if (allTop.find(uid) != allTop.end()) {
                    _pComm->_pUserConfig->updateUserSetting(UserSettingMsg::EM_OPERATOR_CANCEL,
                                                            "kStickJidDic",
                                                            uid,
                                                            "{\"topType\":0,\"chatType\":1}");
                }
            }
        }

        if (restSts && mainVersion != originVersion) {
            restSts = LogicManager::instance()->getDatabase()->setGroupMainVersion(
                          mainVersion);
        }
    }

    return restSts;
}

/**
  * @函数名   getGroupCard
  * @功能描述 获取群名片
  * @参数
     bool
  * @author   cc
  * @date     2018/09/30
  */
bool GroupManager::getGroupCard(const MapGroupCard &groups)
{
    //[{"mucs":[{"version":2, "muc_name" : "de07fb3c853c4ba68f402431ae65d5de@conference.ejabhost1"}], "domain" : "ejabhost1"}]
    std::ostringstream url;
    url << NavigationManager::instance().getHttpHost()
        << "/muc/get_muc_vcard.qunar"
        << "?v=" << DC.getClientVersion()
        << "&p=" << DC.getPlatformStr()
        << "&u=" << DC.getSelfUserId()
        << "&k=" << DC.getServerAuthKey()
        << "&d=" << DC.getSelfDomain();
    nJson objs;
    auto itObj = groups.cbegin();

    for (; itObj != groups.cend(); itObj++) {
        nJson obj;
        obj["domain"] = itObj->first;
        nJson group;

        for (const auto &itG : itObj->second) {
            nJson g;
            g["muc_name"] = itG.GroupId;
            g["version"] = 0;
            group.push_back(g);
        }

        obj["mucs"] = group;
        objs.push_back(obj);
    }

    string postData = objs.dump();
    bool retSts = false;
    string strUrl = url.str();
    std::vector<entity::ImGroupInfo> arGroups;
    auto callback = [strUrl, &retSts, &arGroups](int code,
    const string & responseData) {
        info_log("{0} \n{1}", strUrl, responseData);

        if (code == 200) {
            nJson data = Json::parse(responseData);

            if (data == nullptr) {
                warn_log("parsing json error.{0}", strUrl);
                return;
            }

            bool ret = Json::get<bool>(data, "ret");

            if (ret) {
                nJson jsonGroups = Json::get<nJson>(data, "data");

                for (auto &item : jsonGroups) {
                    nJson mucs = Json::get<nJson>(item, "mucs");

                    for (auto &gobj : mucs) {
                        entity::ImGroupInfo group;
                        group.GroupId = Json::get<string>(gobj, "MN");
                        group.Name = Json::get<string>(gobj, "SN");
                        group.Introduce = Json::get<string>(gobj, "MD");
                        group.Topic = Json::get<string>(gobj, "MT");
                        group.HeaderSrc = Json::get<string>(gobj, "MP");

                        if (!group.HeaderSrc.empty() && group.HeaderSrc.find("http") != 0) {
                            group.HeaderSrc = NavigationManager::instance().getFileHttpHost() + "/" +
                                              group.HeaderSrc;
                        }

                        group.LastUpdateTime = std::strtoll(Json::get<string>(gobj, "UT").data(),
                                                            nullptr, 0);
                        arGroups.push_back(group);
                    }
                }

                retSts = true;
            }
        } else {
            warn_log("请求失败  url:{0}", strUrl);
        }
    };

    if (_pComm) {
        HttpRequest req(strUrl, RequestMethod::POST);
        req.header["Content-Type"] = "application/json;";
        req.body = postData;
        _pComm->addHttpRequest(req, callback);

        if (retSts) {
            retSts = LogicManager::instance()->getDatabase()->updateGroupCard(arGroups);
        }
    }

    return retSts;
}

/**
 * 更新群资料
 */
bool GroupManager::upateGroupInfo(const st::StGroupInfo &groupInfo)
{
    std::ostringstream url;
    url << NavigationManager::instance().getHttpHost()
        << "/muc/set_muc_vcard.qunar"
        << "?v=" << DC.getClientVersion()
        << "&p=" << DC.getPlatformStr()
        << "&u=" << DC.getSelfUserId()
        << "&k=" << DC.getServerAuthKey()
        << "&d=" << DC.getSelfDomain();
    nJson objs;
    nJson obj;
    obj["desc"] = groupInfo.desc;
    obj["muc_name"] = groupInfo.groupId;
    obj["nick"] = groupInfo.name;
    obj["title"] = groupInfo.title;
    objs.push_back(obj);
    string postData = objs.dump();
    //
    string strUrl = url.str();
    std::vector<entity::ImGroupInfo> groups;
    auto callback = [strUrl, &groups](int code, const string & response) {
        if (code == 200) {
            nJson resData = Json::parse(response);

            if (nullptr == resData) {
                error_log("upateGroupInfo json error {0}", response);
                return;
            }

            int ret = Json::get<bool>(resData, "ret");

            if (ret) {
                nJson data = Json::get<nJson>(resData, "data");

                for (auto &item : data) {
                    entity::ImGroupInfo groupInfo;
                    groupInfo.GroupId = Json::get<string>(item, "muc_name");
                    groupInfo.Name = Json::get<string>(item, "show_name");
                    groupInfo.Introduce = Json::get<string>(item, "muc_desc");
                    groupInfo.Topic = Json::get<string>(item, "muc_title");
                    groups.emplace_back(groupInfo);
                }
            } else {
                if (resData.contains("errmsg")) {
                    info_log(Json::get<string>(resData, "errmsg"));
                }
            }

            info_log("update group info success {0}", response);
        } else {
            info_log("update group info error {0}", response);
        }
    };

    if (_pComm) {
        HttpRequest req(strUrl, RequestMethod::POST);
        req.header["Content-Type"] = "application/json;";
        req.body = postData;
        _pComm->addHttpRequest(req, callback);

        if (!groups.empty()) {
            // 更新ui
            {
                auto it = groups.begin();

                for (; it != groups.end(); it++) {
                    std::shared_ptr<st::StGroupInfo> info(new st::StGroupInfo);
                    info->groupId = it->GroupId;
                    info->name = it->Name;
                    info->desc = it->Introduce;
                    info->title = it->Topic;
                    CommMsgManager::onUpdateGroupInfo(info);
                }
            }
            MapGroupCard params;

            for (const auto &it : groups) {
                st::entity::JID jid(it.GroupId);
                params[jid.domainname()].push_back(it);
            }

            getGroupCard(params);
        }
    }

    return !groups.empty();
}

bool GroupManager::updateTopic(const string &groupId,
                               const string &topic)
{
    std::ostringstream url;
    url << NavigationManager::instance().getHttpHost()
        << "/muc/set_muc_vcard.qunar"
        << "?v=" << DC.getClientVersion()
        << "&p=" << DC.getPlatformStr()
        << "&u=" << DC.getSelfUserId()
        << "&k=" << DC.getServerAuthKey()
        << "&d=" << DC.getSelfDomain();
    nJson objs;
    nJson obj;
    obj["muc_name"] = groupId;
    obj["title"] = topic;
    objs.push_back(obj);
    string postData = objs.dump();
    //
    string strUrl = url.str();
    std::vector<entity::ImGroupInfo> groups;
    auto callback = [strUrl, &groups](int code, const string & response) {
        if (code == 200) {
            nJson resData = Json::parse(response);

            if (nullptr == resData) {
                error_log("updateTopic json error {0}", response);
                return;
            }

            int ret = Json::get<bool>(resData, "ret");

            if (ret) {
                nJson data = Json::get<nJson>(resData, "data");

                for (auto &item : data) {
                    entity::ImGroupInfo groupInfo;
                    groupInfo.GroupId = Json::get<string>(item, "muc_name");
                    groupInfo.Name = Json::get<string>(item, "show_name");
                    groupInfo.Introduce = Json::get<string>(item, "muc_desc");
                    groupInfo.Topic = Json::get<string>(item, "muc_title");
                    groups.emplace_back(groupInfo);
                }
            } else {
                if (resData.contains("errmsg")) {
                    info_log(Json::get<string>(resData, "errmsg"));
                }
            }

            info_log("update group info success {0}", response);
        } else {
            info_log("update group info error {0}", response);
        }
    };

    if (_pComm) {
        HttpRequest req(strUrl, RequestMethod::POST);
        req.header["Content-Type"] = "application/json;";
        req.body = postData;
        _pComm->addHttpRequest(req, callback);

        if (!groups.empty()) {
            // 更新ui
            {
                auto it = groups.begin();

                for (; it != groups.end(); it++) {
                    std::shared_ptr<st::StGroupInfo> info(new st::StGroupInfo);
                    info->groupId = it->GroupId;
                    info->name = it->Name;
                    info->desc = it->Introduce;
                    info->title = it->Topic;
                    CommMsgManager::onUpdateGroupInfo(info);
                }
            }
            //            更新群资料需要调接口 否则时间戳不能及时更新
            MapGroupCard params;

            for (const auto &it : groups) {
                st::entity::JID jid(it.GroupId);
                params[jid.domainname()].push_back(it);
            }

            getGroupCard(params);
        }
    }

    return !groups.empty();
}

/**
 * 增量获取群卡片
 */
void GroupManager::getUserIncrementMucVcard()
{
    try {
        //
        long long maxGroupCardVersion = 0;
        LogicManager::instance()->getDatabase()->getGroupCardMaxVersion(
            maxGroupCardVersion);
        //
        std::ostringstream url;
        url << NavigationManager::instance().getHttpHost()
            << "/muc/get_user_increment_muc_vcard.qunar"
            << "?v=" << DC.getClientVersion()
            << "&p=" << DC.getPlatformStr()
            << "&u=" << DC.getSelfUserId()
            << "&k=" << DC.getServerAuthKey()
            << "&d=" << DC.getSelfDomain();
        nJson obj;
        obj["userid"] = DC.getSelfUserId().data();
        obj["lastupdtime"] = std::to_string(maxGroupCardVersion);
        string postData = obj.dump();
        //
        bool retSts = false;
        string strUrl = url.str();
        std::vector<entity::ImGroupInfo> arGroups;
        auto callback = [strUrl, &retSts, &arGroups](int code,
        const string & responseData) {
            if (code == 200) {
                nJson data = Json::parse(responseData);

                if (data == nullptr) {
                    warn_log("parsing json error.{0}", strUrl);
                    return;
                }

                bool ret = Json::get<bool>(data, "ret");

                if (ret) {
                    nJson jsonGroups = Json::get<nJson>(data, "data");

                    for (auto &item : jsonGroups) {
                        entity::ImGroupInfo group;
                        group.GroupId = Json::get<string>(item, "MN");
                        group.Name = Json::get<string>(item, "SN");
                        group.Introduce = Json::get<string>(item, "MD");
                        group.Topic = Json::get<string>(item, "MT");
                        group.HeaderSrc = Json::get<string>(item, "MP");

                        if (!group.HeaderSrc.empty() && group.HeaderSrc.find("http") != 0) {
                            group.HeaderSrc = NavigationManager::instance().getFileHttpHost() + "/" +
                                              group.HeaderSrc;
                        }

                        group.LastUpdateTime = std::strtoll(Json::get<string>(item, "UT").data(),
                                                            nullptr, 0);
                        arGroups.push_back(group);
                    }

                    retSts = true;
                }
            } else {
                debug_log("请求失败  url:{0}", strUrl);
            }
        };

        if (_pComm) {
            HttpRequest req(strUrl, RequestMethod::POST);
            req.header["Content-Type"] = "application/json;";
            req.body = postData;
            _pComm->addHttpRequest(req, callback);

            if (retSts) {
                LogicManager::instance()->getDatabase()->updateGroupCard(arGroups);
            }
        }
    } catch (const std::exception &e) {
        error_log("exception {0}", e.what());
    }
}
