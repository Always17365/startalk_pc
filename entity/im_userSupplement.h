//
// Created by cc on 18-11-6.
//

#ifndef STALK_V2_IM_USERSUPPLEMENT_H
#define STALK_V2_IM_USERSUPPLEMENT_H

#include <string>
#include "include/CommonDefine.h"

/**
 * @brief
 * @author cc
 * @date 2018.11.06
 */
namespace st {
    namespace entity {

        struct ImUserSupplement
        {
            ImUserSupplement()
            {
                XmppId = "";
                UserNo = "";
                UserMood = "";
                UserMoodVersion = 0;
                LeaderId = "";
                PhoneNo = "";
                MailAddr = "";
                ExtendedFlag = 0;
            }

            std::string XmppId;
            std::string UserNo;
            std::string UserMood;
            QInt32      UserMoodVersion;
            std::string LeaderId;
            std::string LeaderName;
            std::string PhoneNo;
            std::string MailAddr;
            QInt64 ExtendedFlag;
        };
    }
}

#endif //STALK_V2_IM_USERSUPPLEMENT_H
