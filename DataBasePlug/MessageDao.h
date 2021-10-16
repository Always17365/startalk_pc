#ifndef MESSAGEDAO_H
#define MESSAGEDAO_H

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "DaoInterface.h"
#include "include/im_enum.h"
#include "entity/IM_Session.h"
#include <vector>
#include <string>
#include "include/CommonStrcut.h"
#include "entity/UID.h"

namespace st {
    namespace entity {
        struct ImMessageInfo;
    }
}
class MessageDao : public DaoInterface{
public:
    explicit MessageDao(st::sqlite::database *sqlDb = nullptr);
    bool creatTable();

public:
    bool creatIndex();

public:
//    bool insertMessageInfo(const st::Entity::ImMessageInfo &imMessageInfo);

    bool bulkInsertMessageInfo(const std::vector<st::entity::ImMessageInfo> &msgList,
                               std::map<st::entity::UID, st::entity::ImSessionInfo> *sessionMap);

    bool bulkUpdateSessionList(std::map<st::entity::UID, st::entity::ImSessionInfo> *sessionMap);

    bool bulkDeleteMessage(const std::vector<std::string> &groupIds);

    bool deleteMessageByMessageId(const std::string &meesageId);

    bool getUserMessage(const long long &time, const std::string &userName,const std::string &realJid,
                        std::vector<st::entity::ImMessageInfo> &msgList);

    long long getMaxTimeStampByChatType(st::Enum::ChatType chatType);
    long long getMaxTimeStamp();

    bool updateMState(const std::string &messageId, const QInt64 &time);

    bool updateReadMask(const std::map<std::string, QInt32> &readMasks);

    bool updateReadMask(const std::map<std::string, QInt64> &readMasks);

    bool getUnreadedMessages(const std::string &messageId, std::vector<std::string> &msgIds);

    bool getGroupMessageLastUpdateTime(const std::string &messageId, QInt64 &time);

    bool
    getGroupUnreadedCount(const std::map<std::string, QInt64> &readMasks, std::map<std::string, int> &unreadedCount);

    bool getMessageByMessageId(const std::string &messageId, st::entity::ImMessageInfo &imMessageInfo);

    // 撤销消息处理
    bool updateRevokeMessage(const std::string &messageId);

    //
    bool getUnreadCountById(const std::string &id,const std::string& realJid, int &count);

    bool getAtCount(const std::string &selfName, const std::string &id, int &count);

    bool getAllUnreadCount(int &count);

    //
    long long getMinUnReadTimeStamp();
    //
    void getMinUnReadTimeStamp(const std::map<std::string, QInt64> &readMasks, std::map<std::string, QInt64 >& minUnReadedTimes);

    void getBeforeImageMessage(const std::string& messageId,
                               std::vector<std::pair<std::string, std::string>> & msgs) ;

    void getNextImageMessage(const std::string& messageId,
                             std::vector<std::pair<std::string, std::string>> & msgs);

    // local message
    void getLocalMessage(const long long &time, const std::string &userId, const std::string &realJid,
                         std::vector<st::entity::ImMessageInfo> &msgList);

    void getFileMessage(const long long &time, const std::string &userId, const std::string &realJid,
                        std::vector<st::entity::ImMessageInfo> &msgList);

    void getImageMessage(const long long &time, const std::string &userId, const std::string &realJid,
                         std::vector<st::entity::ImMessageInfo> &msgList);

    void getLinkMessage(const long long &time, const std::string &userId, const std::string &realJid,
                         std::vector<st::entity::ImMessageInfo> &msgList);

    void getSearchMessage(const long long &time, const std::string &userId, const std::string &realJid,
                          const std::string& searchKey, std::vector<st::entity::ImMessageInfo> &msgList);

    void getAfterMessage(const long long &time, const std::string &userId, const std::string &realJid,
                         std::vector<st::entity::ImMessageInfo>& msgList);

    //
    void updateMessageExtendInfo(const std::string& msgId, const std::string& info);

public:
    // fix type
    void fixMessageType();
    //
    void updateMessageReadFlags(const std::map<std::string, int>& readFlags);
    // add flag (new / old by rool_back flag)
    void addMessageFlag();
};

#endif // MESSAGEDAO_H
