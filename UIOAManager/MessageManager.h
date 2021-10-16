//
// Created by cc on 18-12-29.
//

#ifndef STALK_V2_MESSAGEMANAGER_H
#define STALK_V2_MESSAGEMANAGER_H

#include "EventBus/Object.hpp"
#include "EventBus/EventHandler.hpp"
#include "include/CommonStrcut.h"

class MessageManager : public Object
{
public:
    static bool getOAUiData(std::vector<st::StOAUIData>&oAUIData);

};


#endif //STALK_V2_MESSAGEMANAGER_H
