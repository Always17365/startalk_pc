//
// Created by QITMAC000260 on 2019-07-15.
//

#include "StyleDefine.h"
#include <QPainter>

namespace QTalk {

    StyleDefine* StyleDefine::_defines = nullptr;
    StyleDefine & StyleDefine::instance()
    {
        if (nullptr == _defines)
        {
            static StyleDefine define;
            _defines = &define;
        }
        return *_defines;
    }
    //
}

namespace QTalk {

    enum {FONT_LEVEL_NORMAL = 0, FONT_LEVEL_BIG, FONT_LEVEL_SMALL};
    void setPainterFont(QPainter* painter, int level, int fontSize)
    {
        if(FONT_LEVEL_BIG == level)
            fontSize += 2;
        else if(FONT_LEVEL_SMALL == level)
            fontSize -= 1;
        else{}

        auto font = painter->font();
        font.setWeight(400);
        font.setPixelSize(fontSize);
        painter->setFont(font);
    }
}