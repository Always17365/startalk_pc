#ifndef SESSIONLISTDAO_H
#define SESSIONLISTDAO_H

#include "DaoInterface.h"
#include "include/CommonStrcut.h"
#include <vector>

#ifdef _LINUX
#include <memory>
#endif

namespace st {
    namespace entity {
        struct ImSessionInfo;
    }
}
class SessionListDao : public DaoInterface{
public:
    explicit SessionListDao(st::sqlite::database *sqlDb = nullptr);
    bool creatTable() override;

public:
//    bool insertSessionInfo(const st::Entity::ImSessionInfo &imSessionInfo);
//
//    bool bulkInsertSessionInfo(const std::vector<st::Entity::ImSessionInfo> &sessionList);

    bool bulkDeleteSession(const std::vector<std::string> &groupIds);

    bool bulkremoveSessionMessage(const std::vector<std::string> &groupIds);

//    std::shared_ptr<std::vector<std::shared_ptr<st::Entity::ImSessionInfo>>> QueryImSessionInfos();
    std::shared_ptr<std::vector<std::shared_ptr<st::entity::ImSessionInfo> > > reloadSession();
    //
    void updateUnreadCount();
    //
    void getRecentSession(std::vector<st::StShareSession>& sessions);

    void updateRealJidForFixBug();
    // 修正消息type
    void fixMessageType();
    //
    void clearSelfUnRead(const std::string& xmppId);

public:
    bool deleteAllSession();

//    bool initSession();
//
//    bool initConfigSessions();

//    bool initSessionInfo(std::vector<std::string> *userList);
};

#endif // SESSIONLISTDAO_H
