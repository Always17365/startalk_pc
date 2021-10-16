//
// Created by QITMAC000260 on 2019/10/16.
//

#ifndef STALK_V2_IM_USER_STATUS_MEDAL_H
#define STALK_V2_IM_USER_STATUS_MEDAL_H

#include <string>

namespace st {
    namespace entity {
        struct ImUserStatusMedal
        {
            int medalId = 0;
            std::string userId;
            std::string host;
            int medalStatus = 0;
            int mappingVersion = 0;
            long long updateTime = 0;
        };
    }
}

#endif //STALK_V2_IM_USER_STATUS_MEDAL_H
