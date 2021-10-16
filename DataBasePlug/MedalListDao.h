//
// Created by cc on 2019/10/15.
//

#ifndef STALK_V2_MEDALLISTDAO_H
#define STALK_V2_MEDALLISTDAO_H

#include "DaoInterface.h"
#include <vector>
#include "entity/im_medal_list.h"

/**
* @description: MedalListDao
* @author: cc
* @create: 2019-10-15 14:11
**/
class MedalListDao : public DaoInterface{
public:
    explicit MedalListDao(st::sqlite::database *sqlDb);
    bool creatTable() override;

public:
    void insertMedalList(const std::vector<st::entity::ImMedalList>& medals);
    void getMedalList(std::vector<st::entity::ImMedalList>& medals);
};


#endif //STALK_V2_MEDALLISTDAO_H
