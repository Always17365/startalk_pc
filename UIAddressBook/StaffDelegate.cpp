﻿//
// Created by cc on 18-12-20.
//

#include "StaffDelegate.h"
#include <QPainter>
#include <QEvent>
#include <QFileInfo>
#include "CustomUi/HeadPhotoLab.h"
#include "Util/ui/qimage/qimage.h"

#include "Util/ui/StyleDefine.h"
#include "DataCenter/AppSetting.h"

using namespace st;
StaffDelegate::StaffDelegate(QWidget *parent)
        : QStyledItemDelegate(parent)
{

}

StaffDelegate::~StaffDelegate()
{

}

/**
 *
 * @param painter
 * @param option
 * @param index
 */
void StaffDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing);
    QRect rect = option.rect;
    if (option.state & QStyle::State_Selected)
        painter->fillRect(rect, StyleDefine::instance().getNavSelectColor());
    else
        painter->fillRect(rect, StyleDefine::instance().getNavNormalColor());

    QString strText = index.data(EM_STAFF_DATATYPE_TEXT).toString();
    QString iconPath = index.data(EM_STAFF_DATATYPE_ICONPATH).toString();
    bool hasChild = index.data(EM_STAFF_DATATYPE_HASCHILD).toBool();
    QPixmap icon = st::qimage::loadCirclePixmap(iconPath, 12);

    painter->drawPixmap(QRect(rect.x(), rect.y() + 8, 24, 24), icon);
    painter->setPen(QPen(StyleDefine::instance().getAdrNameFontColor()));
    st::setPainterFont(painter, AppSetting::instance().getFontLevel());
    painter->drawText(QRect(rect.x() + 25, rect.y(), rect.width() - 25, rect.height()), Qt::AlignVCenter, strText);

    painter->restore();
}

/**
 *
 * @param option
 * @param index
 * @return
 */
QSize StaffDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(option.widget->width(), 40);
}

/**
 *
 * @param event
 * @param model
 * @param option
 * @param index
 * @return
 */
bool StaffDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                const QModelIndex &index)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        emit itemClicked(index);
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

