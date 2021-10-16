//
// Created by lihaibin on 2019-07-03.
//

#ifndef STALK_V2_IM_TRANSFER_H
#define STALK_V2_IM_TRANSFER_H

#include <string>

namespace st {
    namespace entity {
        struct ImTransfer
        {
            std::string userId{};
            std::string userName{};
        };
    }
}

#endif //STALK_V2_IM_TRANSFER_H
