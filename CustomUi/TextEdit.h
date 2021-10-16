//
// Created by cc on 18-12-19.
//

#ifndef STALK_V2_TEXTEDIT_H
#define STALK_V2_TEXTEDIT_H

#include <QTextEdit>
#include "customui_global.h"

class CUSTOMUISHARED_EXPORT TextEdit : public QTextEdit
{
public:
    TextEdit(QWidget *parent = nullptr);
    ~TextEdit();

public:
    enum Flag
    {
        EM_NO_FLAGS = 0,
        EM_NO_EDIT  = 1,
        EM_NO_BORDER = 2,
        EM_NO_H_SCROLLBAR = 4,
        EM_NO_V_SCROLLBAR = 8,
        EM_AUTO_JUST_HEIGHT = 16
    };

public:
    void setFlag(int flags = EM_NO_FLAGS);
    void setText(const QString &text);

public:
    void setRowCount(int rowCount);
    void setLineHeight(int h);

private:
    void adjustHeight();
    int getLineCount();

private:
    bool _autoAdjustHeight {false};
    int  _maxRowCount { 999 };
    int  _lineHeight  { 0 };
    int  _flags       { 0 };
};


#endif //STALK_V2_TEXTEDIT_H
