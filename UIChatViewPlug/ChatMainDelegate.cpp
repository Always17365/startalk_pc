//
// Created by cc on 2019/12/27.
//

#include "ChatMainDelegate.h"
#include "entity/im_message.h"
#include "NativeChatStruct.h"
#include "Util/ui/qimage/qimage.h"
#include "Util/ui/StyleDefine.h"
#include "ChatMainWgt.h"
#include <QPainter>
#include <QFile>
#include "MessageItems/TextMessItem.h"
#include "MessageItems/FileSendReceiveMessItem.h"
#include "MessageItems/CommonTrdInfoItem.h"
#include "MessageItems/CodeItem.h"
#include "MessageItems/AudioVideoItem.h"
#include "MessageItems/VideoMessageItem.h"
#include "MessageItems/TipMessageItem.h"
#include "MessageItems/VoiceMessageItem.h"
#include "MessageItems/NoteMessageItem.h"
#include "MessageItems/SystemMessageItem.h"
#include "MessageItems/NoticeMessageItem.h"
#include "MessageItems/MedalMind.h"
#include "MessageItems/MeetingRemindItem.h"
#include "MessageItems/HotLineAnswerItem.h"
#include "MessageItems/HotLineTipItem.h"
#include "MessageItems/EmojiMessItem.h"
#include "MessageItems/ImageMessItem.h"
#include "MessageItems/TextBrowser.h"


ChatMainDelegate::ChatMainDelegate(QMap<QString, MessageItemBase *> &mapItemWgt,
                                   QListView *parent)
    : QStyledItemDelegate(parent), _parentWgt(parent), _mapItems(mapItemWgt)
{
}

//
QString getStrTime(qint64 time)
{
    QString content = "";
    QDateTime dateTime = QDateTime::currentDateTime();
    QInt64 thisDay = dateTime.toMSecsSinceEpoch() -
                     dateTime.time().msecsSinceStartOfDay();

    if (time < thisDay) {
        content = QDateTime::fromMSecsSinceEpoch(time).toString("yyyy-MM-dd hh:mm:ss");
    } else {
        content = QDateTime::fromMSecsSinceEpoch(time).toString("hh:mm:ss");
    }

    return  content;
}


QWidget *ChatMainDelegate::creatWgt(const QModelIndex &index)
{
    auto msg_type = index.data(EM_USER_MSG_TYPE).toInt();
    auto showShare = index.data(EM_USER_SHOW_SHARE).toBool();

    StNetMessageResult info = index.data(EM_USER_INFO).value<StNetMessageResult>();
    QWidget *item = nullptr;

    switch (msg_type) {
    case st::entity::MessageTypeRevoke:
    case st::entity::MessageTypeGroupNotify:
    case st::entity::MessageTypeShock: {
        auto *pItem = new TipMessageItem();
        pItem->setText(info.body);
        return pItem;
    }

    case st::entity::MessageTypeTime: {
        auto time = index.data(EM_USER_MSG_TIME).toLongLong();
        auto *pItem = new TipMessageItem();
        pItem->setText(getStrTime(time));
        return pItem;
    }

    case st::entity::MessageTypeRobotTurnToUser: {
        item = new HotLineTipItem(info);
        break;
    }

    case st::entity::MessageTypeVoice: {
        item = new VoiceMessageItem(info);
        break;
    }

    case st::entity::MessageTypeProduct:
    case st::entity::MessageTypeNote: {
        item = new NoteMessageItem(info);
        break;
    }

    case st::entity::MessageTypeSystem: {
        item = new SystemMessageItem(info);
        break;
    }

    case st::entity::MessageTypeNotice: {
        item = new NoticeMessageItem(info);
        break;
    }

    case st::entity::MessageTypeMedalRemind: {
        item = new MedalMind(info);
        break;
    }

    case st::entity::MessageTypeMeetingRemind: {
        item = new MeetingRemindItem(info);
        break;
    }

    case st::entity::MessageTypeRobotAnswerList: {
        item = new HotLineAnswerItem(info);
        break;
    }

    case st::entity::MessageTypeFile: {
        item = new FileSendReceiveMessItem(info);
        break;
    }

    case st::entity::MessageTypeCommonTrdInfo:
    case st::entity::MessageTypeCommonTrdInfoV2: {
        item =  new CommonTrdInfoItem(info);
        break;
    }

    case st::entity::MessageTypeSourceCode: {
        item =  new CodeItem(info);
        break;
    }

    case st::entity::WebRTC_MsgType_VideoCall:
    case st::entity::WebRTC_MsgType_AudioCall:
    case st::entity::WebRTC_MsgType_Video:
    case st::entity::WebRTC_MsgType_Video_Group: {
        item =  new AudioVideoItem(info);
        break;
    }

    case st::entity::MessageTypeSmallVideo: {
        item =  new VideoMessageItem(info);
        break;
    }

    case st::entity::MessageTypePhoto:
    case st::entity::MessageTypeImageNew: {
        if (info.text_messages.size() == 1) {
            auto message = info.text_messages[0];

            if (message.type == StTextMessage::EM_EMOTICON) {
                item = new EmojiMessItem(info, message.content, {message.imageWidth, message.imageHeight});
                break;
            } else if (info.text_messages[0].type == StTextMessage::EM_IMAGE) {
                item = new ImageMessItem(info, message.imageLink, message.content, {message.imageWidth, message.imageHeight});
                break;
            } else {}
        }
    }

    case st::entity::MessageTypeText:
    case st::entity::MessageTypeGroupAt:
    case st::entity::MessageTypeRobotAnswer:
    default: {
        item =  new TextMessItem(info);
        break;
    }
    }

    //    info_log("create message item {0} -> {1}", info.xmpp_id.toStdString(), info.msg_id.toStdString());
    auto *base = qobject_cast<MessageItemBase *>(item);

    if (!base) {
        return item;
    }

    //

    base->showShareCheckBtn(showShare);
    _mapItems.insert(info.msg_id, base);

    if (info.state) {
        if (info.state & 1) {
            base->setReadState(info.read_flag | 1);
        } else {
            base->setReadState(info.read_flag);
        }
    } else {

    }

    connect(base, SIGNAL(sgSelectItem(bool)), _parentWgt,
            SLOT(onItemCheckChanged(bool)));
    return base;
}


void ChatMainDelegate::dealWidget(const QModelIndex &index)
{
    QWidget *indexWgt = _parentWgt->indexWidget(index);

    if (indexWgt) {
        auto msg_type = index.data(EM_USER_MSG_TYPE).toInt();

        if (st::entity::MessageTypeTime == msg_type) {
            auto time = index.data(EM_USER_MSG_TIME).toLongLong();
            auto *timeItem = qobject_cast<TipMessageItem *>(indexWgt);

            if (timeItem) {
                timeItem->setText(getStrTime(time));
            }
        }

        indexWgt->setFixedWidth(_parentWgt->width());
    } else {
        auto *itemBase = creatWgt(index);
        _parentWgt->setIndexWidget(index, itemBase);
    }
}

void ChatMainDelegate::paint(QPainter *painter,
                             const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing);

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);
    QRect rect = opt.rect;
    painter->fillRect(rect, st::StyleDefine::instance().getMainWindowColor());

    auto *pDelegate = const_cast<ChatMainDelegate *>(this);

    if (pDelegate) {
        pDelegate->dealWidget(index);
    }

    painter->restore();
}

QSize getsize(const std::vector<StTextMessage> &messages, qreal width)
{
    width -= 20;
    auto *pBrowser = new TextBrowser;

    QTextCharFormat f;
    f.setFontLetterSpacingType(QFont::AbsoluteSpacing);
    f.setFontWordSpacing(0);
    f.setFontLetterSpacing(0);

    for (const auto &msg : messages) {
        switch (msg.type) {
        case StTextMessage::EM_TEXT:
            pBrowser->insertPlainText(msg.content);
            break;

        case StTextMessage::EM_IMAGE:
        case StTextMessage::EM_EMOTICON: {
            QString imagePath = msg.content;
            qreal imageWidth = msg.imageWidth;
            qreal imageHeight = msg.imageHeight;

            if (imagePath.isEmpty() || !QFile::exists(imagePath)) {
                imagePath = ":/chatview/image1/defaultImage.png";
                imageWidth = imageHeight = 80;
            }

            if (QPixmap(imagePath).isNull()) {
                QString realPath = st::qimage::getRealImagePath(imagePath);

                if (QPixmap(realPath).isNull()) {
                    imagePath = ":/chatview/image1/defaultImage.png";
                    imageWidth = imageHeight = 80;
                } else {
                    imagePath = realPath;
                }
            }

            QTextImageFormat imageFormat;
            imageFormat.setWidth(imageWidth);
            imageFormat.setHeight(imageHeight);
            imageFormat.setName(imagePath);
            pBrowser->textCursor().insertImage(imageFormat);
            pBrowser->setCurrentCharFormat(f);
            //
            break;
        }

        case StTextMessage::EM_LINK: {
            QString content = msg.content;
            QTextCharFormat linkFormat = pBrowser->textCursor().charFormat();
            linkFormat.setForeground(QBrush(st::StyleDefine::instance().getLinkUrl()));
            linkFormat.setAnchor(true);
            linkFormat.setAnchorHref(msg.content);
#if QT_DEPRECATED_SINCE(5, 13)
            linkFormat.setAnchorNames(QStringList() << msg.content);
#else
            linkFormat.setAnchorName(msg.content);
#endif
            pBrowser->textCursor().insertText(msg.content, linkFormat);

            pBrowser->setCurrentCharFormat(f);
            break;
        }

        case StTextMessage::EM_ATMSG: {
            QString content = QString(DEM_AT_HTML).arg(msg.content);
            pBrowser->insertHtml(content);
            pBrowser->setCurrentCharFormat(f);
            break;
        }

        default:
            break;
        }
    }

    pBrowser->document()->setTextWidth(width);
    QSizeF docSize = pBrowser->document()->size();
    QSize ret((int)width, (int)docSize.height() + 43);
    delete pBrowser;
    return ret;
}

//
QSize ChatMainDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{

    StNetMessageResult info = index.data(EM_USER_INFO).value<StNetMessageResult>();
    auto msg_type = index.data(EM_USER_MSG_TYPE).toInt();

    switch (msg_type) {
    case st::entity::MessageTypeRevoke:
    case st::entity::MessageTypeGroupNotify:
    case st::entity::MessageTypeShock:
    case st::entity::MessageTypeRobotTurnToUser:
    case st::entity::MessageTypeTime:
        return {_parentWgt->width(), 40};
    case st::entity::MessageTypeFile:

    //            return {_parentWgt->width(), 140};
    case st::entity::MessageTypeSourceCode:

    //            return {_parentWgt->width(), 230};
    case st::entity::MessageTypeProduct:
    case st::entity::MessageTypeNote:
    case st::entity::MessageTypeSystem:
    case st::entity::MessageTypeNotice:
    case st::entity::MessageTypeRobotAnswerList:
    case st::entity::MessageTypeMedalRemind:
    case st::entity::MessageTypeMeetingRemind:
    case st::entity::MessageTypeCommonTrdInfo:
    case st::entity::MessageTypeCommonTrdInfoV2:
    case st::entity::WebRTC_MsgType_VideoCall:
    case st::entity::WebRTC_MsgType_AudioCall:
    case st::entity::WebRTC_MsgType_Video:
    case st::entity::WebRTC_MsgType_Video_Group:
    case st::entity::MessageTypeSmallVideo:
    case st::entity::MessageTypeVoice:
    case st::entity::MessageTypeText:
    case st::entity::MessageTypePhoto:
    case st::entity::MessageTypeGroupAt:
    case st::entity::MessageTypeImageNew:
    case st::entity::MessageTypeRobotAnswer:
    default:
        auto *indexWgt = qobject_cast<MessageItemBase *>(_parentWgt->indexWidget(
                                                             index)) ;

        if (indexWgt) {
            return {_parentWgt->width(), indexWgt->itemWdtSize().height()};
        }

        break;
    }

    //

    switch (msg_type) {
    case st::entity::MessageTypeText:
    case st::entity::MessageTypeGroupAt: {
        auto size = getsize(info.text_messages, _parentWgt->width());
        return {_parentWgt->width(), size.height()};
    }

    case st::entity::MessageTypeSmallVideo: {
        return {_parentWgt->width(),
                info.direction == st::entity::MessageDirectionSent ?
                (int)info.video.height + 18 : (int)info.video.height + 30};
    }

    case st::entity::MessageTypeFile: {
        return {_parentWgt->width(), info.direction == st::entity::MessageDirectionSent ? 134 : 122};
    }

    case st::entity::MessageTypeSourceCode:
        return {_parentWgt->width(), 230};
    case st::entity::WebRTC_MsgType_VideoCall:
    case st::entity::WebRTC_MsgType_AudioCall:
    case st::entity::WebRTC_MsgType_Video:
    case st::entity::WebRTC_MsgType_Video_Group: {
        return {_parentWgt->width(), info.direction == st::entity::MessageDirectionSent ? 58 : 70};
    }

    case st::entity::MessageTypeVoice: {
        return {_parentWgt->width(), info.direction == st::entity::MessageDirectionSent ? 48 : 60};
    }

    default: {
        return {_parentWgt->width(), 250};
    }
    }
}
