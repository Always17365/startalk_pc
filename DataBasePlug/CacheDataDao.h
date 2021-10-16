//
// Created by lihaibin on 2019-02-27.
//

#ifndef STALK_V2_CACHEDATADAO_H
#define STALK_V2_CACHEDATADAO_H

#include "DaoInterface.h"
#include "include/CommonDefine.h"
#include <vector>

class CacheDataDao : public DaoInterface
{
public:
    explicit CacheDataDao(st::sqlite::database *sqlDb);
    bool creatTable() override;

public:
    bool insertUserId(const std::string &value);
    bool insertHotLine(std::string value);
    void getHotLines(std::string &hotLines);
    bool isHotlineMerchant(const std::string &xmppid);

public:
    std::string getGroupReadMarkTime();
    bool updateGroupReadMarkTime(const std::string &time);

    std::string getLoginBeforeGroupReadMarkTime();
    bool saveLoginBeforeGroupReadMarkTime(const std::string &time);

public:
    //
    void clear_data_01();
    // new message timestamp
    void insertNewMessageTimestamp(QInt64 time);
    QInt64 getNewMessageTimestamp();
};

#endif //STALK_V2_CACHEDATADAO_H
