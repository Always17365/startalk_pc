//
// Created by cc on 2019-02-28.
//

#ifndef STALK_V2_MedalMind_H
#define STALK_V2_MedalMind_H

#include "MessageItemBase.h"
#include <QLabel>


class MedalMind : public MessageItemBase
{

    Q_OBJECT
public:
    explicit MedalMind(const StNetMessageResult &msgInfo, QWidget *parent = nullptr);

public:
    QSize itemWdtSize() override;

protected:
    void mousePressEvent(QMouseEvent *e) override;

private:
    void init();
    void initLayout();
    void initSendLayout();
    void initReceiveLayout();
    void initContentLayout();

private:
    QLabel *_iconLab   {nullptr};
    QLabel *_titleLab  {nullptr};
    QLabel *_contentLab{nullptr};

    QSize    _headPixSize;
    QMargins _mainMargin;
    QMargins _leftMargin;
    QMargins _rightMargin;
    QSize    _contentSize;

    int _mainSpacing  {0};
    int _leftSpacing  {0};
    int _rightSpacing {0};
    int _nameLabHeight{0};
};


#endif //STALK_V2_MedalMind_H
