//
// Created by cc on 2020/4/3.
//

#ifndef QTALK_V2_PB2JSON_H
#define QTALK_V2_PB2JSON_H

#include <string>
#include "Protobuf/message.pb.h"

namespace QTalk
{
    std::string toJson(const ProtoMessage* message);
}


#endif //QTALK_V2_PB2JSON_H
