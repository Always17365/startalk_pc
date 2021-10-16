//
// Created by cc on 18-12-6.
//

#ifndef STALK_V2_DBCONFIG_H
#define STALK_V2_DBCONFIG_H

#include "DaoInterface.h"

class DbConfig : public DaoInterface
{
public:
    explicit DbConfig(st::sqlite::database * sqlDb = nullptr);
    bool creatTable() override;

public:
    bool getDbVersion(int &version);
    bool setDbVersion(const int& ver);

    bool initVersions();

private:
    bool initGroupMainVersion();
};


#endif //STALK_V2_DBCONFIG_H
