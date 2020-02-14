//
// Created by cc on 2019-06-18.
//

#ifndef QTALK_V2_MACNOTIFICATION_H
#define QTALK_V2_MACNOTIFICATION_H

/**
* @description: MacNotification
* @author: cc
* @create: 2019-06-18 10:46
**/

#include "../include/im_enum.h"
#include "../QtUtil/Utils/Log.h"
#include "ChatViewMainPanel.h"
#include "../Platform/Platform.h"
#include "../QtUtil/Utils/utils.h"

namespace QTalk {
    namespace mac {

            class Notification {
            public:
                static void showNotification(StNotificationParam* param);
            };
    }
}

#endif //QTALK_V2_MACNOTIFICATION_H
