﻿//
// Created by cc on 2019-04-11.
//

#include "ShareMessageFrm.h"
#include <QHBoxLayout>

ShareMessageFrm::ShareMessageFrm(QWidget *parent)
    : QFrame(parent)
{
    setFixedHeight(150);
    _pSelectCount = new QLabel(this);
    _pSelectCount->setObjectName("SelectCountLabel");
    //
    _pShareBtn = new FunctionButton(tr("分享消息"), "ShareBtn", this);
    _pCancelBtn = new QToolButton(this);
    _pCancelBtn->setObjectName("CancelShareBtn");
    _pCancelBtn->setFixedSize(40, 40);
    //
    auto *btnLay = new QHBoxLayout;
    btnLay->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
    btnLay->addWidget(_pShareBtn);
    btnLay->addWidget(_pCancelBtn);
    btnLay->setAlignment(_pCancelBtn, Qt::AlignTop | Qt::AlignHCenter);
    btnLay->setAlignment(_pShareBtn, Qt::AlignTop | Qt::AlignHCenter);
    btnLay->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
    //
    auto mainLay = new QVBoxLayout(this);
    mainLay->setSpacing(10);
    mainLay->addWidget(_pSelectCount, 0);
    mainLay->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Expanding));
    mainLay->addLayout(btnLay, 0);
    mainLay->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Expanding));
    //
    connect(_pShareBtn, &FunctionButton::clicked, this, [this]()
    {
        emit sgShareMessage();
    });
    connect(_pCancelBtn, &QToolButton::clicked, this, [this]()
    {
        emit sgSetShareMessageState(false);
    });
}

ShareMessageFrm::~ShareMessageFrm()
{
}

void ShareMessageFrm::setSelectCount(unsigned int count)
{
    QString text = QString(tr("已选%1条消息")).arg(count);
    _pSelectCount->setText(text);
}
