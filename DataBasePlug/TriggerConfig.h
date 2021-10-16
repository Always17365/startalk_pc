//
// Created by lihaibin on 2019-07-08.
//

#ifndef STALK_V2_TRIGGERCONFIG_H
#define STALK_V2_TRIGGERCONFIG_H

#include "sqlite/database.h"

class TriggerConfig
{
public:
    explicit TriggerConfig(st::sqlite::database *sqlDb = nullptr);
    bool  createUnreadUpdateTrigger();
    bool  createUnreadInserttrigger();

public:
    void modifyUnreadCountTrigger();

private:
    st::sqlite::database *_sqlDb;
};

#endif //STALK_V2_TRIGGERCONFIG_H
