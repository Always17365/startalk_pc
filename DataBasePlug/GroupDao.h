#ifndef GROUPDAO_H
#define GROUPDAO_H

#include <vector>
#include "include/CommonDefine.h"
#include "DaoInterface.h"
#include <memory>

namespace st {
    namespace entity {
        struct ImGroupInfo;
    }
}

class GroupDao : public DaoInterface {
public:
    explicit GroupDao(st::sqlite::database *sqlDb = nullptr);
    bool creatTable() override;

public:
    bool insertGroupInfo(const st::entity::ImGroupInfo &userInfo);

    bool bulkInsertGroupInfo(const std::vector<st::entity::ImGroupInfo> &groups);

    bool bulkDeleteGroup(const std::vector<std::string> &groupIds);

    bool getGroupLastUpdateTime(long long &lastUpdateTime);

    bool setGroupMainVersion(long long version);

    bool updateGroupCard(const std::vector<st::entity::ImGroupInfo> &groups);

    std::shared_ptr<st::entity::ImGroupInfo> getGroupInfoByXmppId(const std::string &xmppid);
    //
    bool getGroupTopic(const std::string &groupId, std::string &groupTopic);
    //
    bool getAllGroup(std::vector<st::entity::ImGroupInfo> &groups);

    //
    bool getGroupCardById(std::shared_ptr<st::entity::ImGroupInfo> &group);

    bool deleteAllGroup();

    void getGroupCardMaxVersion(long long &version);
};

#endif // GROUPDAO_H
