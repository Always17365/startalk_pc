//
// Created by cc on 2019-02-19.
//

#ifndef STALK_V2_BLOCK_DEFINE_H
#define STALK_V2_BLOCK_DEFINE_H

enum {
    atObjectType = 0x1001,
    quoteObjectType = 0x1002,
    imageObjectType = 0x1003,
    linkObjectType = 0x1004
};

enum {
    imagePropertyPath = QTextFormat::UserProperty + 1,
    imagePropertyLink,
    imagePropertyWidth,
    imagePropertyHeight,
    imagePropertyIndex,
    imagePropertyType,
};

#endif //STALK_V2_BLOCK_DEFINE_H
