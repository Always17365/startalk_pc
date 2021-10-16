//
// Created by QITMAC000260 on 2019/10/15.
//

#ifndef STALK_V2_IM_MEDAL_LIST_H
#define STALK_V2_IM_MEDAL_LIST_H

#include <string>

namespace st {
namespace entity {
struct ImMedalList {
    int medalId = -1;
    std::string medalName;
    std::string obtainCondition;
    std::string smallIcon;
    std::string bigLightIcon;
    std::string bigGrayIcon;
    std::string bigLockIcon;
    int status = 0;
};
}
}

#endif //STALK_V2_IM_MEDAL_LIST_H
