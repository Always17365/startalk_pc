//
// Created by cc on 18-12-29.
//

#ifndef STALK_V2_USELESSMESSAGE_H
#define STALK_V2_USELESSMESSAGE_H

#include "EventBus/Object.hpp"
#include "EventBus/Event.hpp"
#include "include/CommonStrcut.h"

class OAUiDataMessage : public Event
{
public:
    OAUiDataMessage(std::vector<st::StOAUIData>&oAUIData)
        :stOAUIData(oAUIData){};
    ~OAUiDataMessage() {};

public:
    bool ret;
    std::vector<st::StOAUIData>&stOAUIData;
};

#endif //STALK_V2_USELESSMESSAGE_H