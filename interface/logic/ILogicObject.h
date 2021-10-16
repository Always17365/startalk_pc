//
// Created by QITMAC000260 on 2019-08-13.
//

#ifndef STALK_V2_ILOGICOBJECT_H
#define STALK_V2_ILOGICOBJECT_H

class ILogicObject {
public:
    ILogicObject() = default;
    virtual ~ILogicObject() = default;
    ILogicObject (const ILogicObject& other) = default;
};

#endif //STALK_V2_ILOGICOBJECT_H
