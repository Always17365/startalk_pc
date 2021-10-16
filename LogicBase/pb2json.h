//
// Created by cc on 2020/4/3.
//

#ifndef STALK_V2_PB2JSON_H
#define STALK_V2_PB2JSON_H

#include <string>
#include "Protobuf/message.pb.h"

namespace st
{
    std::string toJson(const ProtoMessage* message);
}


#endif //STALK_V2_PB2JSON_H
