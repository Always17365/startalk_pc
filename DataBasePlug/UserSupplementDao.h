//
// Created by cc on 18-11-6.
//

#ifndef STALK_V2_USERSUPPLEMENTDAO_H
#define STALK_V2_USERSUPPLEMENTDAO_H

#include "DaoInterface.h"
#include <memory>
#include "entity/im_userSupplement.h"

class UserSupplementDao : public DaoInterface
{
public:
    explicit UserSupplementDao(st::sqlite::database * sqlDb = nullptr);
    bool creatTable() override;

public:
    bool insertOrUpdateUserMood(const std::string& userId, const std::string& userMood, const int& version);

    bool insertOrUpdateUserPhoneNo(const std::string& userId, const std::string& phoneNo);

    bool insertOrUpdateUserSuppl(std::shared_ptr<st::entity::ImUserSupplement> imUserSup);

private:
    int checkRecordCount(const std::string &userId);
};


#endif //STALK_V2_USERSUPPLEMENTDAO_H
