//
// Created by lihaibin on 2019-07-03.
//

#ifndef STALK_V2_IM_QR_GROUP_H
#define STALK_V2_IM_QR_GROUP_H

#include <string>

namespace st {
    namespace entity {
        struct ImQRgroup
        {
            ImQRgroup()
            {
                sid = 0;
                groupname = "";
                groupseq = 0;
                version = 0;
            }

            int sid;
            std::string groupname;
            int groupseq;
            int version;
        };
    }
}

#endif //STALK_V2_IM_QR_GROUP_H
