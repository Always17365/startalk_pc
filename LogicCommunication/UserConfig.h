//
// Created by cc on 18-12-3.
//

#ifndef STALK_V2_USERCONFIG_H
#define STALK_V2_USERCONFIG_H

#include <string>
#include "include/CommonDefine.h"

class Communication;
class UserConfig
{
public:
    explicit UserConfig(Communication* comm);

public:
    void getUserConfigFromServer(bool sendEvt); // sendEvt 在线时发送增量修改
    // update userSttting
    void updateUserSetting(QUInt8 operatorType, const std::string &key, const std::string &subKey,
                           const std::string &val);

private:
    Communication* _pComm;
};


#endif //STALK_V2_USERCONFIG_H
