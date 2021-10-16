//
// Created by cc on 2019/12/02.
//

#include "ImageDelegate.h"
#include <QPainter>
#include <QPen>
#include <QDebug>
#include <QEvent>
#include "../ChatViewMainPanel.h"
#include "Util/ui/StyleDefine.h"
#include "../NetImageLabel.h"
#include "MessageDelegate.h"
#include "DataCenter/Platform.h"

extern ChatViewMainPanel *g_pMainPanel;
ImageDelegate::ImageDelegate(QTableView *parent)
    : QStyledItemDelegate(parent), _parentWgt(parent)
{
}

QWidget *ImageDelegate::creatWgt(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    StImageResult info = index.data(SEARCH_USER_INFO).value<StImageResult>();
    auto *netImage = new NetImageLabel(info.imageLink);
    connect(netImage, &NetImageLabel::clicked, this, [info]()
    {
        QString imagePath = st::GetImagePathByUrl(info.imageLink.toStdString()).data();
        emit g_pMainPanel->showChatPicture(info.msg_id, info.content, info.index);
    });
    return netImage;
}


void ImageDelegate::dealWidget(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QWidget *indexWgt = _parentWgt->indexWidget(index);

    if (indexWgt)
    {
        auto w = _parentWgt->columnWidth(index.column());
        auto h = _parentWgt->rowHeight(index.row());
        indexWgt->setFixedSize(w, h);
    }
    else
    {
        indexWgt = creatWgt(option, index);
        indexWgt->setContentsMargins(10, 10, 10, 10);
        _parentWgt->setRowHeight(index.row(), 105);
        _parentWgt->setIndexWidget(index, indexWgt);
    }
}

void ImageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing);
    QRect rect = option.rect;
    auto data = index.data(SEARCH_USER_ITEM_TYPE);
    painter->fillRect(rect, st::StyleDefine::instance().getSearchNormalColor());

    if(data.isValid())
    {
        int type = data.toInt();

        if(type == 1)
        {
            _parentWgt->setRowHeight(index.row(), 50);
            painter->drawText(rect, Qt::AlignCenter, index.data(Qt::DisplayRole).toString());
        }
        else
        {
            painter->setPen(Qt::NoPen);

            if (option.state & QStyle::State_Selected)
            {
                QPen pen(st::StyleDefine::instance().getImageSelectBorderColor());
                pen.setWidth(2);
                painter->setPen(pen);
                painter->setBrush(st::StyleDefine::instance().getHeadPhotoMaskColor());
            }
            else
                painter->setBrush(st::StyleDefine::instance().getSearchSelectColor());

            painter->drawRoundedRect(rect.x() + 4, rect.y() + 4, rect.width() - 8, rect.height() - 8, 4, 4);
            auto *pThis = const_cast<ImageDelegate *>(this);

            if (pThis)
                pThis->dealWidget(option, index);
        }
    }

    painter->restore();
}
