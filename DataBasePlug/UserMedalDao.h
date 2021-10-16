//
// Created by cc on 2019/10/15.
//

#ifndef STALK_V2_USERMEDALDAO_H
#define STALK_V2_USERMEDALDAO_H

#include "DaoInterface.h"
#include <vector>
#include <set>
#include "entity/im_user_status_medal.h"
#include "include/CommonStrcut.h"

/**
* @description: UserMedalDao
* @author: cc
* @create: 2019-10-15 14:25
**/
class UserMedalDao : public  DaoInterface{
public:
    explicit UserMedalDao(st::sqlite::database *sqlDb);
    bool creatTable() override ;

public:
    void insertMedals(const std::vector<st::entity::ImUserStatusMedal>& medals);
    void getUserMedal(const std::string& xmppId, std::set<st::StUserMedal>& stMedal);
    void getMedalUsers(int medalId, std::vector<st::StMedalUser>& metalUsers);
    void modifyUserMedalStatus(const std::string& userId, int medalId, int status);
};


#endif //STALK_V2_USERMEDALDAO_H
