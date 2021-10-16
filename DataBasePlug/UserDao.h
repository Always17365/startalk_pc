#ifndef USERDAO_H
#define USERDAO_H

#include "DaoInterface.h"
#include <vector>
#include <memory>
#include "include/CommonStrcut.h"

namespace st {
    namespace entity {
        struct ImUserInfo;
    }
}

class UserDao : public DaoInterface{
public:
    explicit UserDao(st::sqlite::database *sqlDb = nullptr);
    bool creatTable() override;

public:
    bool getUserVersion(int &version);

    bool insertUserInfo(const st::entity::ImUserInfo &userInfo);

    bool bulkInsertUserInfo(const std::vector<st::entity::ImUserInfo> &userInfos);

    bool bulkDeleteUserInfo(const std::vector<std::string> &userIds);

    std::shared_ptr<st::entity::ImUserInfo> getUserInfoByXmppId(const std::string &xmppid);

    bool setUserCardInfo(const std::vector<st::StUserCard> &userInfos);

    //
    bool getUserCardInfos(const std::vector<std::string>& arUserIds, std::vector<st::StUserCard> &userInfos);
    bool getUserCardInfos(std::map<std::string, st::StUserCard> &userInfos);

    //
    bool getStructure(std::vector<std::shared_ptr<st::entity::ImUserInfo>> &structure);

    //
    bool getStructureCount(const std::string &strName, int &count);

    //
    bool getStructureMember(const std::string &strName, std::vector<std::string> &arMember);


    void geContactsSession(std::vector<st::StShareSession> &sessions);

public:
    bool addColumn_03();
    void addColumn_04();
    void modDefaultValue_05();
};

#endif // USERDAO_H
