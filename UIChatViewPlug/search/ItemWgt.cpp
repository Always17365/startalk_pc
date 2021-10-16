#include <utility>


//
// Created by cc on 2019-06-26.
//

#include "ItemWgt.h"
#include "../../entity/im_message.h"
#include <QPainter>
#include <QHBoxLayout>
#include <QDateTime>
#include <QLabel>
#include "../ChatViewMainPanel.h"
#include "Util/ui/StyleDefine.h"

extern ChatViewMainPanel *g_pMainPanel;
NameTitleLabel::NameTitleLabel(int dir, QString name, QString time, QWidget *parent)
    : QFrame(parent), _dir(dir), _name(std::move(name)), _time(std::move(time))
{
    setFixedHeight(20);
}

void NameTitleLabel::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    QRect rect = this->contentsRect();

    if(st::entity::MessageDirectionSent == _dir)
        painter.setPen(QColor(0,202,190));
    else
        painter.setPen(QColor(155,155,155));

    painter.drawText(rect, Qt::AlignLeft | Qt::AlignVCenter,
            _name.isEmpty() ? _time : QString("%1  %2").arg(_name, _time));

    return QFrame::paintEvent(e);
}