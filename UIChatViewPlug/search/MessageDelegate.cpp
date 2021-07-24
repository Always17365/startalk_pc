//
// Created by cc on 2019/11/05.
//

#include "MessageDelegate.h"
#include <QListView>
#include "../MessageItems/TextBrowser.h"
#include "../../UICom/StyleDefine.h"
#include "SearchItemWgt.h"
#include "../../entity/im_message.h"
#include <QPainter>

MessageDelegate::MessageDelegate(QListView *parent)
    : QStyledItemDelegate(parent), _parentWgt(parent)
{
}

SearchItemBase *MessageDelegate::creatWgt(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    auto msg_type = index.data(SEARCH_USER_MSG_TYPE).toInt();
    StNetMessageResult info = index.data(SEARCH_USER_INFO).value<StNetMessageResult>();

    switch (msg_type)
    {
        case QTalk::Entity::MessageTypeRevoke:
        case QTalk::Entity::MessageTypeGroupNotify:
        case QTalk::Entity::MessageTypeShock:
            {
                return new SearchTipITem(info);
            }

        case QTalk::Entity::MessageTypeFile:
            {
                auto *item =  new SearchFileITem(info);
                return item;
            }

        case QTalk::Entity::MessageTypeCommonTrdInfo:
        case QTalk::Entity::MessageTypeCommonTrdInfoV2:
            {
                auto *item =  new SearchCommonTrdItem(info);
                return item;
            }

        case QTalk::Entity::MessageTypeSourceCode:
            {
                auto *item =  new SearchCodeItem(info);
                return item;
            }

        case QTalk::Entity::WebRTC_MsgType_VideoCall:
        case QTalk::Entity::WebRTC_MsgType_AudioCall:
        case QTalk::Entity::WebRTC_MsgType_Video:
        case QTalk::Entity::WebRTC_MsgType_Video_Group:
            {
                auto *item =  new SearchAudioVideoItem(info);
                return item;
            }

        case QTalk::Entity::MessageTypeSmallVideo:
            {
                auto *item =  new SearchVideoItem(info);
                return item;
            }

        case QTalk::Entity::MessageTypeText:
        case QTalk::Entity::MessageTypePhoto:
        case QTalk::Entity::MessageTypeGroupAt:
        case QTalk::Entity::MessageTypeImageNew:
        case QTalk::Entity::MessageTypeRobotAnswer:
        default:
            {
                auto *browser = new SearchTextItem(info);
                connect(browser, &SearchTextItem::sgSelectIndex, this, [this, index]()
                {
                    _parentWgt->setCurrentIndex(index);
                });
                return browser;
            }
    }
}


void MessageDelegate::dealWidget(const QStyleOptionViewItem &option, const QModelIndex &index)
{
    QWidget *indexWgt = _parentWgt->indexWidget(index);

    if (indexWgt)
    {
    }
    else
    {
        auto *itemBase = creatWgt(option, index);
        connect(itemBase, &SearchItemBase::sgGetMessageDetail,
                this, &MessageDelegate::sgGetMessageDetail);
        _parentWgt->setIndexWidget(index, itemBase);
    }
}

void MessageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing);
    QRect rect = option.rect;

    if (option.state & QStyle::State_Selected)
        painter->fillRect(rect, QTalk::StyleDefine::instance().getSearchSelectColor());
    else
        painter->fillRect(rect, QTalk::StyleDefine::instance().getSearchNormalColor());

    auto *pThis = const_cast<MessageDelegate *>(this);

    if (pThis)
        pThis->dealWidget(option, index);

    painter->restore();
}

//
QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto size = QStyledItemDelegate::sizeHint(option, index);
    auto msg_type = index.data(SEARCH_USER_MSG_TYPE).toInt();

    switch (msg_type)
    {
        case QTalk::Entity::MessageTypeRevoke:
        case QTalk::Entity::MessageTypeGroupNotify:
        case QTalk::Entity::MessageTypeShock:
            {
                return {0, 80};
            }

        case QTalk::Entity::MessageTypeFile:
            {
                return {0, 100};
            }

        case QTalk::Entity::MessageTypeCommonTrdInfo:
        case QTalk::Entity::MessageTypeCommonTrdInfoV2:
            {
                return {0, 100};
            }

        case QTalk::Entity::MessageTypeSourceCode:
            {
                return {0, 120};
            }

        case QTalk::Entity::WebRTC_MsgType_VideoCall:
        case QTalk::Entity::WebRTC_MsgType_AudioCall:
        case QTalk::Entity::WebRTC_MsgType_Video:
        case QTalk::Entity::WebRTC_MsgType_Video_Group:
            {
                return {0, 80};
            }

        case QTalk::Entity::MessageTypeSmallVideo:
            {
                StNetMessageResult info = index.data(SEARCH_USER_INFO).value<StNetMessageResult>();
                return {0, (int)info.video.height + 20 + 20};
            }

        case QTalk::Entity::MessageTypeText:
        case QTalk::Entity::MessageTypePhoto:
        case QTalk::Entity::MessageTypeGroupAt:
        case QTalk::Entity::MessageTypeImageNew:
        case QTalk::Entity::MessageTypeRobotAnswer:
        default:
            auto *indexWgt = qobject_cast<SearchTextItem *>(_parentWgt->indexWidget(index)) ;

            if (indexWgt)
                return indexWgt->getContentSize(_parentWgt->width());

            break;
    }

    size = QSize(_parentWgt->width(), size.height());
    return {0, qMax(size.height(), 40)};
}
