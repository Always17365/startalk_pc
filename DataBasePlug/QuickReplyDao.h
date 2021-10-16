//
// Created by lihaibin on 2019-06-26.
//

#ifndef STALK_V2_QUICKREPLYDAO_H
#define STALK_V2_QUICKREPLYDAO_H

#include "DaoInterface.h"
#include "include/CommonDefine.h"
#include "entity/im_qr_group.h"
#include "entity/im_qr_content.h"
#include <vector>

class QuickReplyDao : public DaoInterface{

public:
    explicit QuickReplyDao(st::sqlite::database *sqlDb);
    bool creatTable() override;

public:
    void batchInsertQuickReply(const std::string &data);
    void getQuickReplyVersion(QInt64 version[]);

public:
    void getQuickGroups(std::vector<st::entity::ImQRgroup>& groups);
    void getQuickContentByGroup(std::vector<st::entity::IMQRContent>& contents,int id);


};
#endif //STALK_V2_QUICKREPLYDAO_H
