﻿//
// Created by admin on 2019-01-22.
//

#ifndef QTALK_V2_COMMONTRDINFOITEM_H
#define QTALK_V2_COMMONTRDINFOITEM_H

#include "MessageItemBase.h"

#include <QLabel>
#include <QTextEdit>

class CommonTrdInfoEdit : public QTextEdit {
    Q_OBJECT
public:
    explicit CommonTrdInfoEdit(QWidget* parent = nullptr)
        : QTextEdit(parent) {
    }

protected:
    void wheelEvent(QWheelEvent *event) override {};

};

class CommonTrdInfoItem : public MessageItemBase {

Q_OBJECT
public:
    explicit CommonTrdInfoItem(const QTalk::Entity::ImMessageInfo &msgInfo, QWidget *parent = Q_NULLPTR);

public:
    QSize itemWdtSize() override;

Q_SIGNALS:
    void sgDownloadedIcon(const QString&);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void setIcon(const QString& iconPath);

private:
    void init();
    void initLayout();
    void initSendLayout();
    void initReceiveLayout();
    void initContentLayout();

private:
    QLabel *_iconLab;
    QLabel *_titleLab;
    QTextEdit *_contentLab;
    QLabel *_sourceLab;

    QSize _headPixSize;
    QMargins _mainMargin;
    QMargins _leftMargin;
    QMargins _rightMargin;
    QSize _contentSize;
    int _mainSpacing;

    int _leftSpacing;
    int _rightSpacing;
    int _nameLabHeight;

};


#endif //QTALK_V2_COMMONTRDINFOITEM_H
