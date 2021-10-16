//
// Created by cc on 2019/11/05.
//

#include "MessageDelegate.h"
#include <QListView>
#include "../MessageItems/TextBrowser.h"
#include "Util/ui/StyleDefine.h"
#include "SearchItemWgt.h"
#include "entity/im_message.h"
#include <QPainter>

MessageDelegate::MessageDelegate(QListView *parent)
    : QStyledItemDelegate(parent), _parentWgt(parent)
{
}

SearchItemBase *MessageDelegate::creatWgt(const QStyleOptionViewItem &option,
                                          const QModelIndex &index)
{
    auto msg_type = index.data(SEARCH_USER_MSG_TYPE).toInt();
    StNetMessageResult info = index.data(
                                  SEARCH_USER_INFO).value<StNetMessageResult>();

    switch (msg_type) {
    case st::entity::MessageTypeRevoke:
    case st::entity::MessageTypeGroupNotify:
    case st::entity::MessageTypeShock: {
        return new SearchTipITem(info);
    }

    case st::entity::MessageTypeFile: {
        auto *item =  new SearchFileITem(info);
        return item;
    }

    case st::entity::MessageTypeCommonTrdInfo:
    case st::entity::MessageTypeCommonTrdInfoV2: {
        auto *item =  new SearchCommonTrdItem(info);
        return item;
    }

    case st::entity::MessageTypeSourceCode: {
        auto *item =  new SearchCodeItem(info);
        return item;
    }

    case st::entity::WebRTC_MsgType_VideoCall:
    case st::entity::WebRTC_MsgType_AudioCall:
    case st::entity::WebRTC_MsgType_Video:
    case st::entity::WebRTC_MsgType_Video_Group: {
        auto *item =  new SearchAudioVideoItem(info);
        return item;
    }

    case st::entity::MessageTypeSmallVideo: {
        auto *item =  new SearchVideoItem(info);
        return item;
    }

    case st::entity::MessageTypeText:
    case st::entity::MessageTypePhoto:
    case st::entity::MessageTypeGroupAt:
    case st::entity::MessageTypeImageNew:
    case st::entity::MessageTypeRobotAnswer:
    default: {
        auto *browser = new SearchTextItem(info);
        connect(browser, &SearchTextItem::sgSelectIndex, this, [this, index]() {
            _parentWgt->setCurrentIndex(index);
        });
        return browser;
    }
    }
}


void MessageDelegate::dealWidget(const QStyleOptionViewItem &option,
                                 const QModelIndex &index)
{
    QWidget *indexWgt = _parentWgt->indexWidget(index);

    if (indexWgt) {
    } else {
        auto *itemBase = creatWgt(option, index);
        connect(itemBase, &SearchItemBase::sgGetMessageDetail,
                this, &MessageDelegate::sgGetMessageDetail);
        _parentWgt->setIndexWidget(index, itemBase);
    }
}

void MessageDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing);
    QRect rect = option.rect;

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, st::StyleDefine::instance().getSearchSelectColor());
    } else {
        painter->fillRect(rect, st::StyleDefine::instance().getSearchNormalColor());
    }

    auto *pThis = const_cast<MessageDelegate *>(this);

    if (pThis) {
        pThis->dealWidget(option, index);
    }

    painter->restore();
}

//
QSize MessageDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    auto size = QStyledItemDelegate::sizeHint(option, index);
    auto msg_type = index.data(SEARCH_USER_MSG_TYPE).toInt();

    switch (msg_type) {
    case st::entity::MessageTypeRevoke:
    case st::entity::MessageTypeGroupNotify:
    case st::entity::MessageTypeShock: {
        return {0, 80};
    }

    case st::entity::MessageTypeFile: {
        return {0, 100};
    }

    case st::entity::MessageTypeCommonTrdInfo:
    case st::entity::MessageTypeCommonTrdInfoV2: {
        return {0, 100};
    }

    case st::entity::MessageTypeSourceCode: {
        return {0, 120};
    }

    case st::entity::WebRTC_MsgType_VideoCall:
    case st::entity::WebRTC_MsgType_AudioCall:
    case st::entity::WebRTC_MsgType_Video:
    case st::entity::WebRTC_MsgType_Video_Group: {
        return {0, 80};
    }

    case st::entity::MessageTypeSmallVideo: {
        StNetMessageResult info = index.data(
                                      SEARCH_USER_INFO).value<StNetMessageResult>();
        return {0, (int)info.video.height + 20 + 20};
    }

    case st::entity::MessageTypeText:
    case st::entity::MessageTypePhoto:
    case st::entity::MessageTypeGroupAt:
    case st::entity::MessageTypeImageNew:
    case st::entity::MessageTypeRobotAnswer:
    default:
        auto *indexWgt = qobject_cast<SearchTextItem *>(_parentWgt->indexWidget(
                                                            index)) ;

        if (indexWgt) {
            return indexWgt->getContentSize(_parentWgt->width());
        }

        break;
    }

    size = QSize(_parentWgt->width(), size.height());
    return {0, qMax(size.height(), 40)};
}
