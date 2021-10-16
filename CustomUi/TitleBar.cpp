//
// Created by cc on 2019/11/05.
//

#include "TitleBar.h"
#include <QHBoxLayout>
#include <QLabel>

TitleBar::TitleBar(const QString &title, QWidget *controlWgt, QWidget *parent)
    : QFrame(parent)
{
    this->setObjectName("TitleBar");
    auto *titleLay = new QHBoxLayout(this);
    titleLay->setMargin(10);
    _pCloseBtn = new QToolButton(this);
    _pTitleLbl = new QLabel(title, this);
    _pTitleLbl->setObjectName("TitleBarLabel");
    _pTitleLbl->setAlignment(Qt::AlignmentFlag::AlignCenter);
#ifdef _MACOS
    _pCloseBtn->setFixedSize(10, 10);
    titleLay->addWidget(_pCloseBtn);
    _pCloseBtn->setObjectName("gmCloseBtn");
    titleLay->addWidget(_pTitleLbl);
    titleLay->addItem(new QSpacerItem(12, 12));
#else
    titleLay->addWidget(_pTitleLbl);
    _pCloseBtn->setFixedSize(24, 24);
    titleLay->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
    _pCloseBtn->setObjectName("gwCloseBtn");
    titleLay->addWidget(_pCloseBtn);
#endif

    //
    if(nullptr != controlWgt)
        connect(_pCloseBtn, &QToolButton::clicked, this, [controlWgt]()
    {
        controlWgt->setVisible(false);
    });
}

void TitleBar::setLabelText(const QString &text)
{
    if(nullptr != _pTitleLbl)
        _pTitleLbl->setText(text);
}
