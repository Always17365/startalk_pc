﻿//
// Created by cc on 2019-08-13.
//

#include "LogicBase.h"
#include <future>

#include "MessageManager.h"
#include "QTalkSecurity.h"
#include "Util/Log.h"
#include "Util/utils.h"
#include "Util/nJson/nJson.h"
#include "BaseUtil.hpp"
#include "pb2json.h"
#include "Util/threadhelper.h"

#if defined(_MACOS)
    #define DEM_CLIENT_TYPE ClientTypeMac
    #include <unistd.h>
#elif defined(_LINUX)
    #define DEM_CLIENT_TYPE ClientTypeLinux
    #include <unistd.h>
#else
    #include<windows.h>
    #define DEM_CLIENT_TYPE ClientTypePC
#endif

#define DEM_READMASK_GROUPREADED "2"
#define DEM_READMASK_UNREAD "3"
#define DEM_READMASK_READED "4"
#define DEM_READMASK_PROCESSED "7"

#define CONSULT_QCHAT_ID "qchatid"
#define CONSULT_VALUE "4"
#define CONSULT_SERVER_VALUE "5"

using namespace st;
using std::string;

static bool process_ = false;

LogicBase::LogicBase()
    : _isConnected(false)
{
    process_ = true;
    _pMsgListener = new LogicBaseMsgListener(this);
}

LogicBase::~LogicBase()
{
    process_ = false;
    delete _pMsgListener;
    _pMsgListener = nullptr;
#ifndef _LINUX

    if (nullptr != _pStack) {
        delete _pStack;
        //        _pStack = nullptr;
    }

#endif
}

bool LogicBase::tryConnectToServer(const string &userName,
                                   const string &password, const string &host,
                                   int port, bool isNewLogin)
{
    if (_isConnected) {
        warn_log("dump connect ...");
        return true;
    }

    info_log("connect to server : {0} -> {1}", host, port);
    st::entity::JID jid(userName, DC.getClientVersion(),
                        DC.getPlatformStr(), AppSetting::instance().getTestchannel());

    if (nullptr == _pStack) {
        _pStack = new st::Protocol::ProtobufStack(this);
    }

    _pStack->setLoginUser(jid);
    _pStack->setHost(host.c_str());
    _pStack->setPort(port);
    _pStack->setPassword(password.c_str());
    _pStack->setNewLogin(isNewLogin);
    bool res = _pStack->asyncConnect();
    {
        std::lock_guard<st::util::spin_mutex> lock(sm);
        _isConnected = res;
    }
    return _isConnected;
}



string LogicBase::chatRsaEncrypt(const string &value)
{
    return st::Security::chatRsaEncrypt(value);
}

string LogicBase::normalRsaEncrypt(const string &value)
{
    return st::Security::normalRsaEncrypt(value);
}

void LogicBase::onConnectedFailed(const string &reason)
{
    error_log("login failed reason: {0}", reason);
    std::lock_guard<st::util::spin_mutex> lock(sm);
    _isConnected = false;
}

/**
  * @函数名   tcp连接断开
  * @功能描述
  * @参数
  * @author   cc
  * @date     2018/10/23
  */
void LogicBase::onTcpDisconnect(const string &error, bool hadError)
{
    {
        std::lock_guard<st::util::spin_mutex> lock(sm);

        if (!_isConnected && _delayTask && _delayTask->isRuning()) {
            return;
        }
    }
    auto ptr = std::make_shared<ThreadHelper>();
    ptr->run([this, error, hadError]() {
        if (_isConnected) {
            LogicBaseMsgManager::onUpdateTimeStamp();
        }

        if (!hadError) {
            LogicBaseMsgManager::onAuthFailed(error);
        }

        //
        {
            std::lock_guard<st::util::spin_mutex> lock(sm);

            if (_isConnected) {
                _isConnected = false;
            }
        }
        LogicBaseMsgManager::onDisconnectToServer();

        //
        if (process_ && _autoReconnectToServer) {
            auto reconnectFun = [this]() -> bool {
                if (_isConnected)
                {
                    return false;
                }

                StHostInfo info;
                takeHostInfo(&info, _host.data());

                if (info.ip.empty())
                {
                    error_log("check network: no network");
                    return true;
                } else
                {
                    info_log("check network: yes");
                    bool ret = LogicBaseMsgManager::retryConnecToServer();

                    if (!ret) {
                        std::lock_guard<st::util::spin_mutex> lock(sm);
                        this->_delayTask->setDelay(10000);
                    }

                    return true;
                }
            };
            {
                std::lock_guard<st::util::spin_mutex> lock(sm);

                if (nullptr == _delayTask) {
                    _delayTask = new DelayTask(5000, "-- reconnect ", reconnectFun);
                } else {
                    _delayTask->setDelay(5000);
                }

                _delayTask->start();
            }
        }
    });
}

/**
 * 收到错误消息
 */
void LogicBase::recvErrorMessage(const string &fromId,
                                 const XmppMessage &xmppMsg)
{
    auto ptr = std::make_shared<ThreadHelper>();
    ptr->run([ fromId, xmppMsg]() {
#ifdef _MACOS
        pthread_setname_np("recvErrorMessage thread");
#endif
        const MessageBody &body = xmppMsg.body();

        for (const StringHeader &header : body.headers()) {
            // black list
            if (header.key() == "errcode" && header.value() == "406") {
                const string &messageId = xmppMsg.messageid();
                LogicBaseMsgManager::recvBlackListMessage(fromId, messageId);
                LogicManager::instance()->getDatabase()->deleteMessageByMessageId(messageId);
                error_log("error code 406 message id {0}, from {1}", messageId, fromId);
                break;
            }
        }
    });
}

void onRecvGroupMembers(const std::vector<StMessageBody> &bodys,
                        std::map<string, QUInt8> &mapUserRole)
{
    for (auto &body : bodys) {
        string memberid;
        QUInt8 userRole = 3;

        for (const StStringHeader &header : body.headers) {
            if (header.definedKey == StringHeaderTypeJid) {
                memberid = header.value;
            } else if (header.definedKey == StringHeaderTypeAffiliation) {
                if (header.value == "owner") {
                    userRole = 1;
                } else if (header.value == "admin") {
                    userRole = 2;
                }
            } else {
                userRole = 0;
            }
        }

        mapUserRole[memberid] = userRole;
    }
}

void LogicBase::dealBindMsg(const IQMessageEvt &evt)
{
    if (nullptr == _pStack) {
        error_log("_pStack is null");
        //
        return;
    }

    //
    string strUser = evt.body.value;
    st::entity::JID jid(strUser);
    DC.setSelfResource(jid.resources());
    //
    std::map<StringHeaderType, string> infoMap;

    for (const StStringHeader &header : evt.body.headers) {
        infoMap.insert(std::pair<StringHeaderType, string>((
                                                               StringHeaderType)header.definedKey, header.value));
        info_log(" -- {0} : {1}", header.definedKey, header.value);
    }

    // token
    string remoteKey = infoMap.at(StringHeaderType::StringHeaderTypeKeyValue);
    DC.setServerAuthKey(remoteKey);
    // diff server time
    long long serverTime = std::strtoll(infoMap.at(
                                            StringHeaderTypeTimeValue).data(), nullptr, 0);
    time_t now = time(nullptr);
    long long timeDiff = now - serverTime;
    DC.setServerDiffTime(timeDiff);
    // open db
    LogicBaseMsgManager::onDealBind();

    // start recv online message
    if (_isConnected && nullptr != _pStack) {
        _pStack->sendPriority(5);
    }

    // sleep 500 ms
    const std::chrono::milliseconds ms(500);
    std::this_thread::sleep_for(ms);
    //
    LogicBaseMsgManager::synSeverData();
    //
    LogicBaseMsgManager::LoginSuccess(strUser);
}

void LogicBase::onRecvIQMessage(const IQMessageEvt &evt)
{
    if (evt.value == "bind") {
        auto ptr = std::make_shared<ThreadHelper>();
        ptr->run([this, evt]() {
            dealBindMsg(evt);
        });
        return;
    }

    string id;
    {
        std::lock_guard<st::util::spin_mutex> lock(_iq_sm);
        auto itFind = _mapIQMessageId.find(evt.messageId);

        if (itFind != _mapIQMessageId.end()) {
            id = itFind->second;
            _mapIQMessageId.erase(itFind);
        }
    }

    if (id.empty()) {
        return;
    }

    auto ptr = std::make_shared<ThreadHelper>();
    ptr->run([id, evt]() {
        switch (evt.definedKey) {
        case IQKeyResult: {
            if (evt.value == "muc_users") {
                //
                std::map<string, QUInt8> mapUserRole;
                onRecvGroupMembers(evt.bodys, mapUserRole);
                LogicBaseMsgManager::onRecvGroupMembers(id, mapUserRole);
            } else if (evt.value == "create_muc") {
                bool ret = evt.body.value == "success";
                info_log("group:{0} {1}", id, (ret ? " create success " : " create failed "));
                LogicBaseMsgManager::creatGroupResult(id, ret);
            } else if (evt.value == "muc_invite_user_v2") {
                LogicBaseMsgManager::onInviteGroupMembers(id);
            }
            break;
        }

        default:
            break;
        }
    });
}

/**
 * 销毁群处理
 * @param groupId
 */
void LogicBase::destroyGroup(const string &groupId)
{
    IQMessage iq;
    iq.set_key("DESTROY_MUC");
    iq.set_definedkey(IQKeyDestroyMuc);
    iq.set_value(groupId);
    string messageId = st::utils::uuid();
    iq.set_messageid(messageId);
    //
    st::entity::JID jid(groupId);

    if (_pStack) {
        _pStack->sendIQMessage(&iq, &jid);
    }
}

/**
 * 退出群
 * @param groupId
 */
void LogicBase::quitGroup(const string &groupId)
{
    //
    IQMessage iq;
    iq.set_key("DEL_MUC_USER");
    iq.set_definedkey(IQKeyDelMucUser);
    iq.set_messageid(st::utils::uuid());
    // send quit group message
    st::entity::JID jid(groupId);

    if (_pStack) {
        _pStack->sendIQMessage(&iq, &jid);
    }
}

void LogicBase::removeGroupMember(const string &groupId,
                                  const string &nickName,
                                  const string &memberJid)
{
    IQMessage iq;
    iq.set_key("CANCEL_MEMBER");
    iq.set_definedkey(IQKeyCancelMember);
    //    iq.set_value(groupId);
    string messageId = st::utils::uuid();
    iq.set_messageid(messageId);
    MessageBody *body = iq.mutable_body();
    body->set_value("item");
    StringHeader *memberJidHeader = body->add_headers();
    memberJidHeader->set_key("read_jid");
    memberJidHeader->set_definedkey(StringHeaderTypeRealJid);
    memberJidHeader->set_value(memberJid);
    StringHeader *nickHeader = body->add_headers();
    nickHeader->set_key("nick");
    nickHeader->set_definedkey(StringHeaderTypeNick);
    nickHeader->set_value(nickName);
    StringHeader *affiliationHeader = body->add_headers();
    affiliationHeader->set_key("role");
    affiliationHeader->set_definedkey(StringHeaderTypeRole);
    affiliationHeader->set_value("none");
    //
    //    _mapIQMessageId[messageId] = groupId;
    //
    st::entity::JID jid(groupId);

    if (_pStack) {
        _pStack->sendIQMessage(&iq, &jid);
    }
}

void LogicBase::setGroupAdmin(const string &groupId,
                              const string &nickName,
                              const string &memberJid, bool isAdmin)
{
    IQMessage iq;
    iq.set_key(isAdmin ? "SET_MEMBER" : "SET_ADMIN");
    iq.set_definedkey(isAdmin ? IQKeySetAdmin : IQKeySetMember);
    //    iq.set_value(groupId);
    string messageId = st::utils::uuid();
    iq.set_messageid(messageId);
    MessageBody *body = iq.mutable_body();
    body->set_value("item");
    StringHeader *memberJidHeader = body->add_headers();
    memberJidHeader->set_key("read_jid");
    memberJidHeader->set_definedkey(StringHeaderTypeRealJid);
    memberJidHeader->set_value(memberJid);
    StringHeader *nickHeader = body->add_headers();
    nickHeader->set_key("nick");
    nickHeader->set_definedkey(StringHeaderTypeNick);
    nickHeader->set_value(nickName);
    StringHeader *affiliationHeader = body->add_headers();
    affiliationHeader->set_key("affiliation");
    affiliationHeader->set_definedkey(StringHeaderTypeAffiliation);
    affiliationHeader->set_value(isAdmin ? "member" : "admin");
    //
    //    _mapIQMessageId[messageId] = groupId;
    //
    st::entity::JID jid(groupId);

    if (_pStack) {
        _pStack->sendIQMessage(&iq, &jid);
    }
}

//
void LogicBase::getGroupMemberById(const string &groupId)
{
    if (_isConnected && nullptr != _pStack) {
        IQMessage iq;
        iq.set_definedkey(IQKeyGetMucUser);
        string msgId = st::utils::uuid();
        iq.set_messageid(msgId);
        {
            std::lock_guard<st::util::spin_mutex> lock(_iq_sm);
            _mapIQMessageId[msgId] = groupId;
        }
        st::entity::JID jid(groupId);
        _pStack->sendIQMessage(&iq, &jid);
    }
}

//
void LogicBase::inviteGroupMembers(std::vector<string> &users,
                                   const string &groupId)
{
    IQMessage message;
    string msgId = st::utils::uuid();
    message.set_messageid(msgId);
    message.set_key("MUC_INVITE_V2");
    message.set_value(groupId);
    auto item = users.begin();

    while (item != users.end()) {
        MessageBody *body = message.add_bodys();
        body->set_value("invite");
        StringHeader *header = body->add_headers();
        header->set_key("jid");
        header->set_value(*item);
        item++;
    }

    {
        std::lock_guard<st::util::spin_mutex> lock(_iq_sm);
        _mapIQMessageId[msgId] = groupId;
    }

    st::entity::JID groupJid(groupId);

    if (_pStack) {
        _pStack->sendIQMessage(&message, &groupJid);
    }
}

/**
 *
 * @param groupId
 * @param groupName
 */
void LogicBase::createGroup(const string &groupId)
{
    IQMessage iqMessage;
    string msgId = st::utils::uuid();
    iqMessage.set_messageid(msgId);
    iqMessage.set_key("CREATE_MUC");
    iqMessage.set_value(groupId);
    st::entity::JID jid(groupId);
    {
        std::lock_guard<st::util::spin_mutex> lock(_iq_sm);
        _mapIQMessageId[msgId] = groupId;
    }

    if (_pStack) {
        _pStack->sendIQMessage(&iqMessage, &jid);
    }
}

/**
 *
 */
void LogicBase::sendHeartbeat()
{
    IQMessage iq;
    iq.set_key("PING");
    iq.set_definedkey(IQKeyPing);
    iq.set_messageid(st::utils::uuid());

    if (_pStack) {
        _pStack->sendIQMessage(&iq, nullptr);
    }
}

void LogicBase::onRecvPresenceMessage(const ProtoMessage &message,
                                      const PresenceMessage &presenceMsg)
{
    string json = st::toJson(&message);
    info_log(json);
    int definedkey = presenceMsg.definedkey();
    debug_log("recv presenceMsg definedkey:{0}, value: {1}, body: {2}, categorytype: {3}",
              definedkey, presenceMsg.value(),
              presenceMsg.body().value(), presenceMsg.categorytype());
    auto presenceFunc = [ message, presenceMsg]() {
#ifdef _MACOS
        pthread_setname_np("communication onRecvPresenceMessage thread");
#endif

        // todo 换成接口
        if (presenceMsg.value() == "update_muc_vcard") {
            std::shared_ptr<st::StGroupInfo> info(new st::StGroupInfo);
            st::entity::JID gJid(message.from());
            info->groupId = gJid.basename();
            auto headers = presenceMsg.body().headers();
            std::for_each(headers.begin(),
            headers.end(), [info](const StringHeader & header) {
                switch (header.definedkey()) {
                case StringHeaderTypeNick:
                    info->name = header.value();
                    break;

                case StringHeaderTypeTitle:
                    info->title = header.value();
                    break;

                case StringHeaderTypePic:
                    info->headSrc = header.value();
                    break;

                case StringHeaderTypeVersion:
                    info->version = (int)std::strtof(header.value().c_str(), nullptr);
                    break;

                default:
                    break;
                }

                debug_log("update_muc_vcard-> definedkey: {0} value:{1}", header.definedkey(),
                          header.value());
            });
            st::entity::ImGroupInfo im_gInfo;
            im_gInfo.GroupId = info->groupId;
            im_gInfo.Name = info->name;
            im_gInfo.Introduce = info->title;
            im_gInfo.HeaderSrc = info->headSrc;
            // 先更新ui
            LogicBaseMsgManager::onUpdateGroupInfo(info);
            // 更新db
            std::vector<st::entity::ImGroupInfo> groups;
            groups.push_back(im_gInfo);
            LogicManager::instance()->getDatabase()->updateGroupCard(groups);
        } else if (presenceMsg.value() == "update_user_status") {
            //       不再处理其他客户端发来的用户状态
            st::entity::JID jid(message.from());

            if (jid.resources() == DC.getSelfResource()) {
                const auto &body = presenceMsg.body();
                string status;

                for (const auto &header : body.headers()) {
                    if ("show" == header.key()) {
                        status = header.value();
                    }
                }

                if (!status.empty()) {
                    LogicBaseMsgManager::onSwitchUserStatus(status);
                }
            }
        } else if (presenceMsg.value() == "notify") {
            switch (presenceMsg.categorytype()) {
            case CategoryConfigSync: { //
                string bodyVal = presenceMsg.body().value();
                nJson data = Json::parse(bodyVal);

                if (data == nullptr) {
                    error_log("presenceMsg notify CategoryConfigSync json paring error");
                    return;
                }

                string source = Json::get<string >(data, "resource");

                if (source != DC.getSelfResource()) {
                    LogicBaseMsgManager::onUserConfigChanged();
                }

                break;
            }

            case CategoryOrganizational: {
                // 获取组织架构
                string json = st::toJson(&message);
                info_log(">>> CategoryOrganizational {}", json);
                LogicBaseMsgManager::onStaffChanged();
                break;
            }

            // 反馈日志
            case CategoryClientSpecialNotice: {
                string bodyVal = presenceMsg.body().value();
                nJson data = Json::parse(bodyVal);

                if (data != nullptr) {
                    if (data.contains("uploadLog")) {
                        int ret = Json::get<int>(data, "uploadLog");

                        if (ret) {
                            LogicBaseMsgManager::onFeedLog("@@#@@");
                        }
                    }
                }

                break;
            }

            case CategoryMedalListSync: { //勋章列表同步
                string bodyVal = presenceMsg.body().value();
                LogicBaseMsgManager::onMedalListChanged();
                break;
            }

            case CategoryMedalUserStatusListSync: { //用户勋章列表同步
                string bodyVal = presenceMsg.body().value();
                LogicBaseMsgManager::onUserMedalChanged();
                break;
            }
            case CategoryTickUser: {
	            LogicBaseMsgManager::goBackLoginWnd();
            	break;
            }
            case CategoryNavigation:
            case CategoryOnlineClientSync:
            default:
                break;
            }
        } else if (presenceMsg.value() == "user_join_muc") {
            string bodyVal = presenceMsg.body().value();
            string groupId = st::entity::JID(message.from()).basename();
            // 加入群通知
            auto headers = presenceMsg.body().headers();
            string memberId, domain;
            int affiliation = 3;
            //
            std::for_each(headers.begin(), headers.end(), [&memberId, &affiliation,
                       & domain](const StringHeader & header) {
                StringHeaderType headerType = header.definedkey();

                if (headerType == StringHeaderTypeJid) {
                    memberId = header.value();
                } else if (headerType == StringHeaderTypeAffiliation) {
                    const string &aff = header.value();

                    if (aff == "owner") {
                        affiliation = 1;
                    } else if (aff == "admin") {
                        affiliation = 2;
                    }
                } else if (headerType == StringHeaderTypeDomain) {
                    domain = header.value();
                }
            });

            if (!memberId.empty() && !domain.empty()) {
                string jid = memberId + "@" + domain;
                LogicBaseMsgManager::onUserJoinGroup(groupId, jid, affiliation);
            }
        }
        // 销毁群处理
        else if (presenceMsg.value() == "destory_muc") {
            string groupId = message.from();
            info_log("destory_muc:groupId:{0} presenceValue:{1}", groupId,
                     presenceMsg.value());
            st::entity::JID jid(groupId);
            groupId = jid.basename();
            LogicBaseMsgManager::onDestroyGroup(groupId, true);
            //
            std::vector<string> groupIds;
            groupIds.push_back(groupId);
            LogicManager::instance()->getDatabase()->bulkDeleteGroup(groupIds);
        } else if (presenceMsg.value() == "del_muc_register") {
            string groupId = message.from();
            st::entity::JID jid(groupId);
            string delUser;

            for (const StringHeader &header : presenceMsg.body().headers()) {
                switch (header.definedkey()) {
                case StringHeaderTypeDeleleJid:
                    delUser = header.value();
                    break;

                default:
                    break;
                }
            }

            debug_log(groupId + " talk " + delUser);
            // 自己被踢
            groupId = jid.basename();

            if (delUser == DC.getSelfXmppId()) {
                std::vector<string> groupIds;
                groupIds.push_back(groupId);
                LogicManager::instance()->getDatabase()->bulkDeleteGroup(groupIds);
                //
                LogicBaseMsgManager::onDestroyGroup(jid.basename(), false);
            }
            // 其他人被踢
            else {
                if (!delUser.empty()) {
                    LogicBaseMsgManager::onRemoveGroupMember(groupId, delUser);
                }
            }
        }
        // 邀请进群
        else if (presenceMsg.value() == "invite_user") {
            const string &groupId = message.from();
            st::entity::JID jid(groupId);
            LogicBaseMsgManager::onInviteGroupMembers(jid.basename());
            const MessageBody &body = presenceMsg.body();

            try {
                if (body.value() == "invite_info") {
                    auto headers = presenceMsg.body().headers();
                    string memberId;
                    int affiliation = 3;
                    std::for_each(headers.begin(),
                    headers.end(), [&memberId](const StringHeader & header) {
                        StringHeaderType headerType = header.definedkey();

                        if (headerType == StringHeaderTypeInviteJid) {
                            memberId = header.value();
                        }
                    });

                    if (!memberId.empty()) {
                        LogicBaseMsgManager::onUserJoinGroup(jid.basename(), memberId, affiliation);
                    }
                }
            } catch (const std::exception &e) {
                error_log("invite_user error {0}", e.what());
            }
        } else if (presenceMsg.value() == "forbidden_words") {
            const auto& body = presenceMsg.body();
            const auto& groupId = message.from();
            bool ok = false;

            for (auto &h : body.headers()) {
                if (h.definedkey() == StringForbiddenWords) {
                    ok = h.value() == "true";
                }
            }

            LogicBaseMsgManager::forbiddenWordGroupState(groupId, ok);
        }
    };
    auto ptr = std::make_shared<ThreadHelper>();
    ptr->run(presenceFunc);
}

void LogicBase::switchUserStatus(const string &status)
{
    if (_isConnected && nullptr != _pStack) {
        static string sstatus;

        if (sstatus == status) {
            return;
        } else {
            sstatus = status;
        }

        //
        PresenceMessage message;
        message.set_key("status");
        message.set_value("user_update_status");
        //        message.set_definedkey(PresenceKeyType::PresenceKeyPriority);
        message.set_messageid(st::utils::uuid());
        MessageBody *msgBody = message.mutable_body();
        {
            StringHeader *header = msgBody->add_headers();
            header->set_key("show");
            header->set_value(status);
        }
        {
            StringHeader *header = msgBody->add_headers();
            header->set_key("priority");
            header->set_value("5");
        }
        ProtoMessage package;
        package.set_signaltype(SignalTypePresence);
        package.set_from(_pStack->getLoginUser()->fullname());
        package.set_message(message.SerializeAsString());
        _pStack->sendProtobufMessage(&package);
    }
}

/**
* 转发消息
* @param messageId
*/
void LogicBase::forwardMessage(const string &messageId,
                               const std::map<string, int> &users)
{
    // 获取原消息内容
    st::entity::ImMessageInfo imMessageInfo;
    LogicManager::instance()->getDatabase()->getMessageByMessageId(messageId,
            imMessageInfo);

    //发送
    for (const auto &user : users) {
        // 转化为xmppmessage
        XmppMessage xMessage;
        xMessage.set_messagetype(imMessageInfo.Type);
        xMessage.set_clienttype(DEM_CLIENT_TYPE);
        xMessage.set_clientversion(atoi(DC.getClientVersion().c_str()));
        MessageBody *msgBody = xMessage.mutable_body();
        msgBody->set_value(imMessageInfo.Content);

        if (!imMessageInfo.ExtendedInfo.empty()) {
            StringHeader *header = msgBody->add_headers();
            header->set_definedkey(StringHeaderTypeExtendInfo);
            header->set_value(imMessageInfo.ExtendedInfo);
        }

        ProtoMessage pMessage;
        auto id = DC.getSelfXmppId();
        pMessage.set_from(id);
        pMessage.set_to(user.first);
        pMessage.set_signaltype(user.second == st::Enum::TwoPersonChat ?
                                SignalTypeChat : SignalTypeGroupChat);
        xMessage.set_messageid(st::utils::uuid());
        pMessage.set_message(xMessage.SerializeAsString());

        if (_isConnected && nullptr != _pStack) {
            _pStack->sendProtobufMessage(&pMessage);
        }

        // 自己处理
        StringHeader *header = msgBody->add_headers();
        header->set_definedkey(StringHeaderTypeCarbon);
        header->set_value("true");
        pMessage.set_from(user.first);
        pMessage.set_to(DC.getSelfXmppId());
        time_t now = time(0);
        xMessage.set_receivedtime((now - DC.getServerDiffTime()) * 1000);
        pMessage.set_message(xMessage.SerializeAsString());
        pMessage.set_sendjid(DC.getSelfXmppId());
        pMessage.set_realfrom(_pStack->getLoginUser()->basename());
        receiveChatMessage(pMessage);
    }
}


// 收到撤销消息处理
void LogicBase::recvRevokeMessage(const string &peerId,
                                  const MessageBody xmppBody, QInt64 time)
{
    //"{\"messageId\":\"809546b6-e8c4-4669-8243-2c2f020ffa95\",\"message\":\"revoke a message\",\"fromId\":\"chaocc.wang@ejabhost1\"}"
    nJson bodyInfo = Json::parse(xmppBody.value());

    if (nullptr != bodyInfo) {
        string messageId = Json::get<string>(bodyInfo, "messageId");
        LogicManager::instance()->getDatabase()->updateRevokeMessage(messageId);
        st::entity::ImMessageInfo imMessageInfo;
        LogicManager::instance()->getDatabase()->getMessageByMessageId(messageId,
                imMessageInfo);
        string realJid = imMessageInfo.RealJid;
        string fromId = Json::get<string>(bodyInfo, "fromId");
        st::entity::JID jid(peerId);
        LogicBaseMsgManager::updateRevokeMessage(st::entity::UID(jid.basename(),
                                                                 realJid), fromId, messageId, time);
    } else {
        warn_log("recvRevokeMessage and body is null??");
    }
}

void
LogicBase::sendRovokeMessage(const st::entity::UID &peerId,
                             const string &msgFrom, const string &messageId, const QInt8 &chatType)
{
    // 更新db
    LogicManager::instance()->getDatabase()->updateRevokeMessage(messageId);
    //
    nJson msgBody;
    msgBody["messageId"] = messageId;
    msgBody["message"] = "revoke a message";
    msgBody["fromId"] = msgFrom.data();
    string bodyVal = msgBody.dump();
    XmppMessage xMessage;
    //consult 撤销单独消息类型
    MessageBody *messageBody = xMessage.mutable_body();

    if (chatType == st::Enum::Consult
        || chatType == st::Enum::ConsultServer) {
        xMessage.set_messagetype(MessageTypeConsultRevoke);
        StringHeader *h1 = messageBody->add_headers();
        h1->set_definedkey(StringHeaderTypeChatId);
        h1->set_value(std::to_string(chatType));
        StringHeader *h2 = messageBody->add_headers();
        h2->set_key(CONSULT_QCHAT_ID);
        h2->set_value(std::to_string(chatType));
        StringHeader *h3 = messageBody->add_headers();
        h3->set_definedkey(StringHeaderTypeChannelId);
        nJson jsonObj;
        jsonObj["cn"] = "consult";
        jsonObj["d"] = "send";
        jsonObj["userType"] = "usr";
        h3->set_value(jsonObj.dump());
    } else {
        xMessage.set_messagetype(MessageTypeRevoke);
    }

    xMessage.set_clienttype(DEM_CLIENT_TYPE);
    xMessage.set_clientversion(0);
    xMessage.set_messageid(messageId);
    messageBody->set_value(bodyVal);
    ProtoMessage pMessage;
    pMessage.set_options(0);
    pMessage.set_from(msgFrom);
    pMessage.set_to(peerId.usrId());
    pMessage.set_realto(peerId.realId());
    pMessage.set_signaltype(SignalTypeRevoke);
    pMessage.set_message(xMessage.SerializeAsString());

    if (_isConnected && nullptr != _pStack) {
        _pStack->sendProtobufMessage(&pMessage);
    }
}

/**
  * @函数名
  * @功能描述 解析发送数据并入库
  * @参数
  * @date 2018.9.26
  */
void LogicBase::parseSendMessageIntoDb(const st::entity::ImMessageInfo
                                       &message, const int messageType)
{
    string messageId = message.MsgId;               // 消息ID
    string messageContent = message.Content;       // 消息内容
    QInt64 recvTime =
        message.LastUpdateTime;                   // 消息接收时间
    string messageTo = message.To;
    string realJid = message.From;
    int clienttype = DEM_CLIENT_TYPE;
    string strFrom = message.From;
    // 定义数据库实体
    //    st::Entity::ImSessionInfo sessionInfo;
    st::entity::ImMessageInfo msgInfo;

    switch (message.ChatType) {
    case st::Enum::TwoPersonChat:
        //二人消息
    {
        // 往message 表里面写
        msgInfo.MsgId = messageId;
        msgInfo.XmppId = messageTo;
        msgInfo.ChatType = st::Enum::TwoPersonChat;
        msgInfo.Platform = clienttype;//
        msgInfo.From = strFrom;
        msgInfo.To = messageTo;
        msgInfo.Content = messageContent;
        msgInfo.Type = messageType;
        msgInfo.State = 0;//
        msgInfo.Direction = 1;//
        msgInfo.ReadedTag = 0;
        msgInfo.LastUpdateTime = recvTime;
        msgInfo.MessageRaw = "";//
        msgInfo.RealJid = messageTo;
        msgInfo.ExtendedInfo = message.ExtendedInfo;//

        if (messageType == MessageTypeFile) {
            //const S_FileMessage &fe = static_cast<const S_FileMessage &>(e);
            //msgInfo.ExtendedFlag = fe.filePath;//
        } else {
            msgInfo.ExtendedFlag = "";//
        }

        msgInfo.BackupInfo = message.BackupInfo;
        LogicManager::instance()->getDatabase()->insertMessageInfo(msgInfo);
    }
    break;

    case st::Enum::Consult:
        //二人消息
    {
        msgInfo.MsgId = messageId;
        msgInfo.XmppId = messageTo;
        msgInfo.ChatType = st::Enum::Consult;
        msgInfo.Platform = clienttype;//
        msgInfo.From = strFrom;
        msgInfo.To = messageTo;
        msgInfo.Content = messageContent;
        msgInfo.Type = messageType;
        msgInfo.State = 0;//
        msgInfo.Direction = 1;//
        msgInfo.ReadedTag = 0;
        msgInfo.LastUpdateTime = recvTime;
        msgInfo.MessageRaw = "";//
        msgInfo.RealJid = messageTo;
        msgInfo.ExtendedInfo = message.ExtendedInfo;//

        if (messageType != MessageTypeFile) {
            msgInfo.ExtendedFlag = "";//
        }

        msgInfo.BackupInfo = "";
        LogicManager::instance()->getDatabase()->insertMessageInfo(msgInfo);
    }
    break;

    case st::Enum::ConsultServer: {
        msgInfo.MsgId = messageId;
        msgInfo.XmppId = messageTo;
        msgInfo.ChatType = st::Enum::ConsultServer;
        msgInfo.Platform = clienttype;//
        msgInfo.From = strFrom;
        msgInfo.To = messageTo;
        msgInfo.Content = messageContent;
        msgInfo.Type = messageType;
        msgInfo.State = 0;//
        msgInfo.Direction = 1;//
        msgInfo.ReadedTag = 0;
        msgInfo.LastUpdateTime = recvTime;
        msgInfo.MessageRaw = "";//
        msgInfo.RealJid = message.RealJid;
        msgInfo.ExtendedInfo = message.ExtendedInfo;//

        if (messageType != MessageTypeFile) {
            msgInfo.ExtendedFlag = "";//
        }

        msgInfo.BackupInfo = "";
        LogicManager::instance()->getDatabase()->insertMessageInfo(msgInfo);
    }
    break;

    case st::Enum::GroupChat: {
        msgInfo.MsgId = messageId;
        msgInfo.XmppId = messageTo;
        msgInfo.ChatType = st::Enum::GroupChat;
        msgInfo.Platform = clienttype;//
        msgInfo.From = realJid;
        msgInfo.To = messageTo;
        msgInfo.Content = messageContent;
        msgInfo.Type = messageType;
        msgInfo.State = 0;//
        msgInfo.Direction = 1;//
        msgInfo.ReadedTag = 0;
        msgInfo.LastUpdateTime = recvTime;
        msgInfo.MessageRaw = "";//
        msgInfo.RealJid = messageTo;
        msgInfo.ExtendedInfo = message.ExtendedInfo;//
        msgInfo.ExtendedFlag = "";//
        msgInfo.BackupInfo = message.BackupInfo;
        LogicManager::instance()->getDatabase()->insertMessageInfo(msgInfo);
    }
    break;

    default:
        break;
    }
}

/**
  * @函数名
  * @功能描述 解析接收数据并入库
  * @参数
  * @date 2018.9.26
  */
void LogicBase::parseReceiveMessageIntoDb(const ProtoMessage &protoMsg,
                                          const XmppMessage &xmppMsg)
{
    int msgType = xmppMsg.messagetype();                       // 消息类型
    const string &messageId = xmppMsg.messageid();        // 消息ID
    string messageContent = xmppMsg.body().value();       // 消息内容
    QInt64 recvTime = xmppMsg.receivedtime();                  // 消息接收时间
    const string &messageTo = st::entity::JID(protoMsg.to()).basename();
    const string &realJid = protoMsg.realfrom();
    int clienttype = xmppMsg.clienttype();
    const string &messageFrom = protoMsg.from();
    // 定义数据库实体
    st::entity::JID destJid(messageFrom);
    string strFrom = destJid.basename();
    st::entity::ImMessageInfo msgInfo;

    if (!xmppMsg.has_body() || xmppMsg.body().headers_size() <= 0) {
        return;
    }

    bool isCarbon = false;
    string extendInfo;
    string backupInfo;
    string qchatId;

    for (const StringHeader &header : xmppMsg.body().headers()) {
        if (header.definedkey() == StringHeaderTypeCarbon) {
            isCarbon = header.value() == "true";
        } else if (header.definedkey() == StringHeaderTypeExtendInfo) {
            extendInfo = header.value();
        } else if (header.definedkey() == StringHeaderTypeBackupInfo) {
            backupInfo = header.value();
        } else if (header.key() == "qchatid") {
            qchatId = header.value();
        }
    }

    switch (protoMsg.signaltype()) {
    case SignalTypeChat:
    case SignalTypeHeadline:
    case SignalTypeSubscription: {
        msgInfo.MsgId = messageId;
        msgInfo.XmppId = strFrom;
        msgInfo.ChatType = st::Enum::TwoPersonChat;
        msgInfo.Platform = clienttype;//
        msgInfo.From = isCarbon ? messageTo : strFrom;
        msgInfo.To = isCarbon ? strFrom : messageTo;
        msgInfo.Content = messageContent;
        msgInfo.Type = msgType;
        msgInfo.State = 1;//
        msgInfo.ReadedTag = 1;

        if (isCarbon) {
            msgInfo.Direction = true;
        } else {
            msgInfo.Direction = (destJid.username() == DC.getSelfUserId());    //
        }

        msgInfo.LastUpdateTime = recvTime;
        msgInfo.MessageRaw = "";//
        msgInfo.RealJid = strFrom;
        msgInfo.ExtendedInfo = extendInfo;//
        msgInfo.ExtendedFlag = "";//
        msgInfo.BackupInfo = backupInfo;

        //

        if (protoMsg.signaltype() == SignalTypeHeadline) {
            strFrom = "SystemMessage@ejabhost1";
            msgInfo.XmppId = strFrom;
            msgInfo.From = strFrom;
            msgInfo.RealJid = strFrom;
            msgInfo.Direction = 0;
            msgInfo.ChatType = st::Enum::System;
        }

        LogicManager::instance()->getDatabase()->insertMessageInfo(msgInfo);
        break;
    }

    case SignalTypeConsult: {
        st::Enum::ChatType chatType{};
        string RealJid;

        if (isCarbon) {
            if (qchatId == CONSULT_VALUE) {
                chatType = st::Enum::Consult;
                st::entity::JID from(protoMsg.from());
                RealJid = from.basename();
            } else if (qchatId == CONSULT_SERVER_VALUE) {
                chatType = st::Enum::ConsultServer;
                st::entity::JID realTo(protoMsg.realto());
                RealJid = realTo.basename();
            }
        } else {
            if (qchatId == CONSULT_VALUE) {
                chatType = st::Enum::ConsultServer;
                RealJid = protoMsg.realfrom();
            } else if (CONSULT_SERVER_VALUE == qchatId) {
                chatType = st::Enum::Consult;
                RealJid = protoMsg.from();
            } else {}
        }

        msgInfo.MsgId = messageId;
        msgInfo.XmppId = strFrom;
        msgInfo.ChatType = (int)chatType;
        msgInfo.Platform = clienttype;//
        msgInfo.From = isCarbon ? messageTo : strFrom;
        msgInfo.To = isCarbon ? strFrom : messageTo;
        msgInfo.Content = messageContent;
        msgInfo.Type = msgType;
        msgInfo.State = 1;//
        msgInfo.ReadedTag = 1;

        if (isCarbon) {
            msgInfo.Direction = true;
        } else {
            msgInfo.Direction = (destJid.username() == DC.getSelfUserId());    //
        }

        msgInfo.LastUpdateTime = recvTime;
        msgInfo.MessageRaw = "";//
        msgInfo.RealJid = RealJid;
        msgInfo.ExtendedInfo = extendInfo;//
        msgInfo.ExtendedFlag = "";//
        msgInfo.BackupInfo = backupInfo;
        LogicManager::instance()->getDatabase()->insertMessageInfo(msgInfo);
    }
    break;

    case SignalTypeGroupChat: {
        msgInfo.MsgId = messageId;
        msgInfo.XmppId = strFrom;
        msgInfo.ChatType = st::Enum::GroupChat;
        msgInfo.Platform = clienttype;//
        msgInfo.From = realJid;
        msgInfo.To = messageTo;
        msgInfo.Content = messageContent;
        msgInfo.Type = msgType;
        msgInfo.State = 1;//
        msgInfo.ReadedTag = 1;
        st::entity::JID realJID(realJid);

        if (isCarbon) {
            msgInfo.Direction = true;
        } else {
            msgInfo.Direction = (realJID.username() == DC.getSelfUserId());    //
        }

        msgInfo.LastUpdateTime = recvTime;
        msgInfo.MessageRaw = "";//
        msgInfo.RealJid = strFrom;
        msgInfo.ExtendedInfo = extendInfo;//
        msgInfo.ExtendedFlag = "";//
        msgInfo.BackupInfo = backupInfo;
        LogicManager::instance()->getDatabase()->insertMessageInfo(msgInfo);
    }
    break;

    default:
        break;
    }
}

void LogicBase::sendMessage(const st::entity::ImMessageInfo &message)
{
    long long clientVersion = DC.getClientNumVerison();

    if (!message.AutoReply) {
        parseSendMessageIntoDb(message, message.Type);
    }

    XmppMessage xMessage;
    xMessage.set_messagetype(message.Type);
    xMessage.set_clienttype(DEM_CLIENT_TYPE);
    xMessage.set_clientversion(clientVersion);
    xMessage.set_messageid(message.MsgId);
    MessageBody *msgBody = xMessage.mutable_body();
    msgBody->set_value(message.Content);

    // add backupinfo
    if (!message.BackupInfo.empty()) {
        StringHeader *header = msgBody->add_headers();
        header->set_definedkey(StringHeaderTypeBackupInfo);
        header->set_value(message.BackupInfo);
    }

    if (!message.ExtendedInfo.empty()) {
        StringHeader *header = msgBody->add_headers();
        header->set_definedkey(StringHeaderTypeExtendInfo);
        header->set_value(message.ExtendedInfo);
    }

    if (message.AutoReply) {
        StringHeader *header = msgBody->add_headers();
        header->set_key("auto_reply");
        header->set_value("true");
    }

    if (message.ChatType == st::Enum::Consult
        || message.ChatType == st::Enum::ConsultServer) {
        StringHeader *h1 = msgBody->add_headers();
        h1->set_definedkey(StringHeaderTypeChatId);
        h1->set_value(std::to_string(message.ChatType));
        StringHeader *h2 = msgBody->add_headers();
        h2->set_key(CONSULT_QCHAT_ID);
        h2->set_value(std::to_string(message.ChatType));
        StringHeader *h3 = msgBody->add_headers();
        h3->set_definedkey(StringHeaderTypeChannelId);
        nJson jsonObj;
        jsonObj["cn"] = "consult";
        jsonObj["d"] = "send";
        jsonObj["userType"] = "usr";
        h3->set_value(jsonObj.dump());
    }

    ProtoMessage pMessage;
    pMessage.set_from(message.From);
    pMessage.set_to(message.To);

    switch (message.ChatType) {
    case st::Enum::TwoPersonChat:
        pMessage.set_signaltype(SignalTypeChat);
        break;

    case st::Enum::GroupChat:
        pMessage.set_signaltype(SignalTypeGroupChat);
        break;

    case st::Enum::Consult:
    case st::Enum::ConsultServer:
        pMessage.set_signaltype(SignalTypeConsult);
        pMessage.set_realfrom(message.From);
        pMessage.set_realto(message.RealJid);
        break;

    default:
        break;
    }

    pMessage.set_message(xMessage.SerializeAsString());

    if (_isConnected && nullptr != _pStack) {
        _pStack->sendProtobufMessage(&pMessage);
    }
}

/**
 * 置阅读状态
 * @param userId
 * @param realJid
 * @param chatType
 */
void LogicBase::dealReadedMessage(const string &messageId,
                                  const string &userId, QUInt8 chatType)
{
    try {
        //
        if (st::Enum::System == chatType ||
            st::Enum::TwoPersonChat == chatType ||
            st::Enum::Consult == chatType ||
            st::Enum::ConsultServer == chatType) {
            std::vector<string> arMsgIds;
            LogicManager::instance()->getDatabase()->getUnreadedMessages(messageId,
                    arMsgIds);

            if (arMsgIds.empty()) {
                warn_log("deal read mask empty u:{0} id:{1}", userId, messageId);
                arMsgIds.push_back(messageId);
                return;
            }

            std::map<string, QInt32> readMasks;

            for (const string &msgId : arMsgIds) {
                debug_log("deal read mask unreaded id:{0}", msgId);
                readMasks[msgId] = 0x02;
            }

            LogicManager::instance()->getDatabase()->updateReadMask(readMasks); //已读
            //
            sendSignalReadMask(arMsgIds, DEM_READMASK_READED, userId);
        } else {
            QInt64 lastTime = 0;
            LogicManager::instance()->getDatabase()->getGroupMessageLastUpdateTime(
                messageId, lastTime);

            if (lastTime <= 0) {
                time_t now = time(0);
                lastTime = (now - DC.getServerDiffTime() - 3600 * 48) * 1000;
            }

            //
            std::map<string, QInt64> mapIds;
            mapIds[userId] = lastTime;
            //
            sendGroupReadMask(mapIds, DEM_READMASK_GROUPREADED, userId);
        }
    } catch (std::exception &e) {
        error_log(e.what());
    }
}

void LogicBase::sendSignalReadMask(const std::vector<string> &ids,
                                   const string &mask,
                                   const string &messageTo)
{
    if (!_isConnected) {
        return;
    }

    nJson jMsgIds;
    st::entity::JID jid(messageTo);
    string barename = jid.basename();

    for (const auto &id : ids) {
        nJson obj {{"id", id}};
        jMsgIds.push_back(obj);
    }

    const string val = jMsgIds.dump();
    XmppMessage xMessage;
    MessageBody *body = xMessage.mutable_body();
    body->set_value(val);
    StringHeader *maskHeader = body->add_headers();
    maskHeader->set_key("read_type");
    maskHeader->set_definedkey(StringHeaderTypeReadType);
    maskHeader->set_value(mask);
    StringHeader *extendHeader = body->add_headers();
    extendHeader->set_key("extendInfo");
    extendHeader->set_definedkey(StringHeaderTypeExtendInfo);
    extendHeader->set_value(barename);
    xMessage.set_messageid(st::utils::uuid());
    xMessage.set_messagetype(0);
    xMessage.set_clientversion(0);
    xMessage.set_clienttype(DEM_CLIENT_TYPE);
    ProtoMessage pMessage;
    pMessage.set_from(_pStack->getLoginUser()->basename());
    pMessage.set_to(barename);
    pMessage.set_signaltype(SignalTypeReadmark);
    pMessage.set_message(xMessage.SerializeAsString());

    if (_pStack) {
        _pStack->sendProtobufMessage(&pMessage);
    }
}

void LogicBase::sendGroupReadMask(const std::map<string, QInt64> &ids,
                                  const string &mask,
                                  const string &messageTo)
{
    if (!_isConnected) {
        return;
    }

    auto it = ids.begin();
    //
    int sedGroupReadMaskCount = 0;

    for (; it != ids.end(); it++) {
        sedGroupReadMaskCount++;
        nJson jMsgIds;
        nJson obj;
        st::entity::JID jid(it->first);
        string barename = jid.basename();
        obj["id"] = jid.username().c_str();
        obj["domain"] = jid.domainname();
        obj["t"] = it->second;
        jMsgIds.push_back(obj);
        string val = jMsgIds.dump();
        XmppMessage xMessage;
        MessageBody *body = xMessage.mutable_body();
        body->set_value(val);
        StringHeader *maskHeader = body->add_headers();
        maskHeader->set_key("read_type");
        maskHeader->set_definedkey(StringHeaderTypeReadType);
        maskHeader->set_value(mask);
        StringHeader *extendHeader = body->add_headers();
        extendHeader->set_key("extendInfo");
        extendHeader->set_definedkey(StringHeaderTypeExtendInfo);
        extendHeader->set_value(barename);
        xMessage.set_messageid(st::utils::uuid());
        xMessage.set_messagetype(0);
        xMessage.set_clientversion(0);
        xMessage.set_clienttype(DEM_CLIENT_TYPE);
        ProtoMessage pMessage;
        pMessage.set_from(_pStack->getLoginUser()->basename());
        pMessage.set_to(_pStack->getLoginUser()->basename());
        pMessage.set_signaltype(SignalTypeReadmark);
        pMessage.set_message(xMessage.SerializeAsString());

        //
        if (_pStack) {
            _pStack->sendProtobufMessage(&pMessage);
        }
    }

    if (sedGroupReadMaskCount > 1) {
        warn_log("it can not be happen! sendGroupReadMask run :{0} times!",
                 sedGroupReadMaskCount);
    }
}

/**
  * @函数名   recvOnlinesReadMask
  * @功能描述 阅读标志消息处理
  * @参数
  * @author   cc
  * @date     2018/10/25
  */
void LogicBase::recvOnlinesReadMask(const ProtoMessage message)
{
    XmppMessage xmppMsg;
    MessageBody body;

    if (xmppMsg.ParseFromString(message.message())) {
        body = xmppMsg.body();
    }

    debug_log("收到更新阅读状态消息 " + body.value());
    nJson jsonMsgId = Json::parse(body.value());

    if (jsonMsgId == nullptr) {
        error_log("json paring error");
        return;
    }

    string xmppid;
    string realJid;
    string ext;
    std::map<string, QInt32> readMasks;
    std::map<string, QInt64> groupReadMasks;

    for (auto &item : jsonMsgId) {
        if (!item.contains( "id")) {
            continue;
        }

        string msgid(Json::get<string>(item, "id"));
        string mask;

        for (const StringHeader &header : body.headers()) {
            if (header.definedkey() == StringHeaderTypeReadType ||
                header.key() == "read_type") {
                mask = header.value();
            } else if (header.definedkey() == StringHeaderTypeExtendInfo ||
                       header.key() == "extendInfo") {
                ext = header.value();
            }
        }

        if (DEM_READMASK_GROUPREADED == mask) {
            xmppid = ext;
        } else {
            if (xmppid.empty()) {
                st::entity::ImMessageInfo msgInfo;
                LogicManager::instance()->getDatabase()->getMessageByMessageId(msgid, msgInfo);
                xmppid = msgInfo.XmppId;
                realJid = msgInfo.RealJid;
            }
        }

        debug_log("recv read mask readed id:{0} mask:{1}", msgid, mask);

        if (DEM_READMASK_GROUPREADED == mask) {
            string domain = Json::get<string >(item, "domain");
            QInt64 t = Json::get<QInt64>(item, "t");
            LogicManager::instance()->getDatabase()->updateGroupReadMarkTime(std::to_string(
                        t));
            groupReadMasks[msgid + "@" + domain] = t;
        } else if (DEM_READMASK_UNREAD == mask) {
            readMasks[msgid] = 1;
        } else if (DEM_READMASK_READED == mask) {
            readMasks[msgid] = 2;
        } else if (DEM_READMASK_PROCESSED == mask) {
            readMasks[msgid] = 4;
        }
    }

    if (!readMasks.empty()) {
        LogicManager::instance()->getDatabase()->updateReadMask(readMasks);
        LogicBaseMsgManager::updateSignalChatReadState(xmppid, realJid, readMasks);
    }

    if (!groupReadMasks.empty()) {
        std::map<string, int> unreadedCount;
        LogicManager::instance()->getDatabase()->getGroupUnreadedCount(groupReadMasks,
                unreadedCount);
        LogicManager::instance()->getDatabase()->updateReadMask(groupReadMasks);

        if (!xmppid.empty() && unreadedCount[xmppid] > 0) {
            LogicBaseMsgManager::updateGroupChatReadState(xmppid, unreadedCount);
        }
    }
}

/**
  * @函数名   recvMState
  * @功能描述 服务器收到消息处理
  * @参数
  * @author   cc
  * @date     2018/10/25
  */
void LogicBase::recvOnlineMState(const string &messageid,
                                 const QInt64 &serverTime)
{
    // update db
    LogicManager::instance()->getDatabase()->updateMState(messageid, serverTime);
    // send message to uodate ui
    st::entity::ImMessageInfo msgInfo;
    LogicManager::instance()->getDatabase()->getMessageByMessageId(messageid,
            msgInfo);
    LogicBaseMsgManager::updateMState(msgInfo.XmppId, msgInfo.RealJid, messageid,
                                      serverTime);
}


/**
  * @函数名   receiveMessage
  * @功能描述 接收到消息
  * @参数
  * @author   cc
  * @date     2018/09/19
  */
void LogicBase::receiveChatMessage(ProtoMessage &message)
{
    if (_pStack == nullptr) {
        return;
    }

    XmppMessage xMessage;
    bool isOk = xMessage.ParseFromString(message.message());

    if (isOk) {
        auto ptr = std::make_shared<ThreadHelper>();
        ptr->run([ = ]() {
#ifdef _MACOS
            pthread_setname_np("communication receive message thread");
#endif
            {
                const MessageBody &body = xMessage.body();
                bool isCarbon = false;
                string extendInfo;
                string backupInfo;
                string qchatId;
                string realJid;
                bool autoReply = false;

                for (const StringHeader &header : body.headers()) {
                    if (header.definedkey() == StringHeaderTypeCarbon) {
                        isCarbon = header.value() == "true";
                    } else if (header.definedkey() == StringHeaderTypeExtendInfo) {
                        extendInfo = header.value();
                    } else if (header.definedkey() == StringHeaderTypeBackupInfo) {
                        backupInfo = header.value();
                    } else if (header.key() == "qchatid") {
                        qchatId = header.value();
                    } else if (header.key() == "auto_reply") {
                        autoReply = header.value() == "true";
                    }
                }

                st::Enum::ChatType chatType{};

                switch (message.signaltype()) {
                case SignalTypeChat:
                    chatType = st::Enum::TwoPersonChat;
                    break;

                case SignalTypeHeadline:
                    chatType = st::Enum::System;
                    break;

                case SignalTypeGroupChat:
                    chatType = st::Enum::GroupChat;
                    break;

                case SignalTypeSubscription:
                    chatType = st::Enum::Robot;
                    break;

                case SignalTypeConsult: {
                    if (isCarbon) {
                        if (CONSULT_VALUE == qchatId) {
                            chatType = st::Enum::Consult;
                            st::entity::JID from(message.from());
                            realJid = from.basename();
                        } else if (CONSULT_SERVER_VALUE == qchatId) {
                            chatType = st::Enum::ConsultServer;
                            st::entity::JID realTo(message.realto());
                            realJid = realTo.basename();
                        }
                    } else {
                        if (qchatId == CONSULT_VALUE) {
                            chatType = st::Enum::ConsultServer;
                            realJid = message.realfrom();
                        } else if (CONSULT_SERVER_VALUE == qchatId) {
                            chatType = st::Enum::Consult;
                            realJid = message.from();
                        }
                    }

                    break;
                }

                default:
                    break;
                }

                //
                debug_log("收到聊天消息 id:{0}  内容:{1} 消息类型{2}",
                          xMessage.messageid(), body.value(), xMessage.messagetype());

                //
                if (!isCarbon && chatType == st::Enum::TwoPersonChat) {
                    std::vector<string> ids;
                    ids.push_back(xMessage.messageid());
                    sendSignalReadMask(ids, DEM_READMASK_UNREAD, message.from());
                }

                if (!autoReply) {
                    parseReceiveMessageIntoDb(message, xMessage);
                }

                //
                st::entity::ImMessageInfo msgEntity;
                msgEntity.ChatType = chatType;
                msgEntity.MsgId = xMessage.messageid();
                msgEntity.Content = body.value();
                msgEntity.SendJid = message.from();
                msgEntity.RealJid = realJid;

                if (isCarbon) {
                    msgEntity.RealFrom = _pStack->getLoginUser()->basename();
                } else {
                    if (chatType == st::Enum::GroupChat) {
                        msgEntity.RealFrom = message.realfrom();
                    } else if (chatType == st::Enum::System) {
                        msgEntity.RealFrom = "SystemMessage@ejabhost1";
                        msgEntity.SendJid = msgEntity.RealFrom;
                    } else {
                        msgEntity.RealFrom = message.from();
                    }
                }

                msgEntity.From = msgEntity.RealFrom;
                msgEntity.LastUpdateTime = xMessage.receivedtime();
                msgEntity.Direction = st::entity::JID(msgEntity.RealFrom).basename() ==
                                      _pStack->getLoginUser()->basename()
                                      ? st::entity::MessageDirectionSent
                                      : st::entity::MessageDirectionReceive;
                msgEntity.ExtendedInfo = extendInfo;
                msgEntity.BackupInfo = backupInfo;
                msgEntity.Type = xMessage.messagetype();
                msgEntity.State = 2;
                msgEntity.ReadedTag = 1;
                msgEntity.AutoReply = autoReply;
                R_Message e;
                e.message = msgEntity;
                e.chatType = chatType;
                e.userId = message.from();
                e.time = msgEntity.LastUpdateTime;
                e.isCarbon = isCarbon;
                LogicBaseMsgManager::onRecvChatMessage(e);
            }
        });
    }
}

bool LogicBase::sendReportMessage(const string &, string &)
{
    return false;
}

/**
 *
 */
bool LogicBase::sendReportMail(const std::vector<string> &to_users,
                               const string &strSub,
                               const string &body,
                               bool isHtml, string &error)
{
    return false;
}

/**
 *
 * @param peerId
 * @param extendInfo
 */
void LogicBase::recvWebRtc(const string &peerId, const XmppMessage &msg)
{
    string extendInfo;
    bool isCarbon = false;

    for (const auto &header : msg.body().headers()) {
        if (header.definedkey() == StringHeaderTypeExtendInfo) {
            extendInfo = header.value();
        } else if (header.definedkey() == StringHeaderTypeCarbon) {
            isCarbon = true;
        } else {}
    }

    if (!extendInfo.empty()) {
        LogicBaseMsgManager::onRecvWebRtcCommand(msg.messagetype(),
                                                 st::entity::JID(peerId).basename(), extendInfo, isCarbon);
    }
}

void LogicBase::sendWebRtcCommand(int msgType, const string &peerId,
                                  const string &extendInfo)
{
    XmppMessage xMessage;
    xMessage.set_messageid(st::utils::uuid());
    xMessage.set_clienttype(DEM_CLIENT_TYPE);
    xMessage.set_clientversion(0);
    xMessage.set_messagetype(msgType);
    auto *body = xMessage.mutable_body();
    body->set_value("video command");
    auto *header = body->add_headers();
    header->set_definedkey(StringHeaderTypeExtendInfo);
    header->set_value(extendInfo);
    ProtoMessage pMessage;
    pMessage.set_options(0);
    pMessage.set_from(_pStack->getLoginUser()->fullname());
    pMessage.set_to(peerId);
    pMessage.set_signaltype(SignalTypeWebRtc);
    pMessage.set_message(xMessage.SerializeAsString());

    if (_pStack) {
        _pStack->sendProtobufMessage(&pMessage);
    }
}

//
void LogicBase::stopTask()
{
    std::lock_guard<st::util::spin_mutex> lock(sm);

    if (nullptr != _delayTask) {
        delete _delayTask;
        _delayTask = nullptr;
    }
}

//
void LogicBase::startAutoReconnectToServer(const string &host)
{
    _autoReconnectToServer = true;
    _host = host.empty() ? "baidu.com" : host;
}

void LogicBase::forbiddenWord(const std::string &groupId, bool status)
{
    IQMessage iq;
    iq.set_definedkey(IQKeySetForbiddenWords);
    iq.set_messageid(st::utils::uuid());
    iq.set_value("forbidden_words");
    MessageBody *body = iq.mutable_body();
    body->set_value("forbidden_words");
    StringHeader *memberJidHeader = body->add_headers();
    memberJidHeader->set_definedkey(StringForbiddenWords);
    memberJidHeader->set_value(status ? "true" : "false");
    // send quit group message
    st::entity::JID jid(groupId);

    if (_pStack) {
        _pStack->sendIQMessage(&iq, &jid);
    }
}

void LogicBase::getForbiddenWordState(const std::string &groupId)
{
    auto id = st::utils::uuid();
    IQMessage iq;
    iq.set_definedkey(IQKeyGetForbiddenWords);
    iq.set_messageid(id);
    iq.set_value("forbidden_words");
    MessageBody *body = iq.mutable_body();
    body->set_value("forbidden_words");
    // send quit group message
    st::entity::JID jid(groupId);

    if (_pStack) {
        {
            std::lock_guard<st::util::spin_mutex> lock(_iq_sm);
            _mapIQMessageId[id] = groupId;
        }
        _pStack->sendIQMessage(&iq, &jid);
    }
}
