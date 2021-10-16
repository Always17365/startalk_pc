//
// Created by cc on 2019/11/05.
//

#ifndef STALK_V2_TITLEBAR_H
#define STALK_V2_TITLEBAR_H

#include <QFrame>
#include <QLabel>
#include <QToolButton>
#include "customui_global.h"

/**
* @description: TitleBar
* @author: cc
* @create: 2019-11-05 11:34
**/
class CUSTOMUISHARED_EXPORT TitleBar : public QFrame {
public:
    explicit TitleBar(const QString& title, QWidget* controlWgt = nullptr, QWidget* parent = nullptr);

public:
    QToolButton* getCloseBtn(){ return _pCloseBtn; };
    void setLabelText(const QString& text);

private:
    QToolButton* _pCloseBtn;
    QLabel*      _pTitleLbl{};
};


#endif //STALK_V2_TITLEBAR_H
