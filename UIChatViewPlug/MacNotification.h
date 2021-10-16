//
// Created by cc on 2019-06-18.
//

#ifndef STALK_V2_MACNOTIFICATION_H
#define STALK_V2_MACNOTIFICATION_H

/**
* @description: MacNotification
* @author: cc
* @create: 2019-06-18 10:46
**/

#include "include/im_enum.h"
#include "Util/Log.h"
#include "ChatViewMainPanel.h"
#include "DataCenter/Platform.h"
#include "Util/utils.h"

namespace st {
    namespace mac {

            class Notification {
            public:
                static void showNotification(StNotificationParam* param);
            };
    }
}

#endif //STALK_V2_MACNOTIFICATION_H
