//
// Created by cc on 18-12-18.
//

#ifndef STALK_V2_IM_FRIEND_LIST_H
#define STALK_V2_IM_FRIEND_LIST_H

namespace st {
    namespace Entity {

        struct IMFriendList
        {
            IMFriendList()
            {
                UserId = "";
                XmppId = "";
                Version = 0;
                ExtendedFlag = "";
            }
            std::string UserId;
            std::string XmppId;
            int         Version;
            std::string ExtendedFlag;
        };
    }
}

#endif //STALK_V2_IM_FRIEND_LIST_H
