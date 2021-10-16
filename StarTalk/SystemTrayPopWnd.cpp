//
// Created by cc on 2020/1/13.
//

#include "SystemTrayPopWnd.h"
#include "CustomUi/ImageLabel.h"
#include "Util/ui/qimage/qimage.h"
#include "CustomUi/Line.h"
#include "Util/ui/StyleDefine.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QScrollBar>
#include <QEvent>
#include <QDebug>

/** **/
ImageTextLabel::ImageTextLabel(const QString &imagePath, const QString &text, QWidget *parent)
    :QFrame(parent)
{
    _path = imagePath;
    _text = text;
}

void ImageTextLabel::paintEvent(QPaintEvent *e) {

    const QRect& rect = this->rect();

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    QFontMetricsF fm(painter.font());
    auto textWidth = fm.width(_text);
    int imageSize = 25;
    auto contentWidth = textWidth + imageSize + 5;
    auto x = (int)(rect.width() - contentWidth) / 2;
    {
        auto dpi = st::qimage::dpi();
        auto pixmap = st::qimage::loadImage(_path, false, true, imageSize * dpi);
        painter.drawPixmap({x, rect.y() + (rect.height() - imageSize) / 2, imageSize, imageSize}, pixmap);
    }

    painter.setPen(QColor(102,102,102));
    QTextOption option;
    option.setAlignment(Qt::AlignVCenter);
    painter.drawText(QRectF(x + imageSize + 5, 0, contentWidth, rect.height()),  _text, option);
    QFrame::paintEvent(e);
}

void ImageTextLabel::mousePressEvent(QMouseEvent *e) {
    emit clicked();
    QFrame::mousePressEvent(e);
}

/** **/
SystemTrayDelegate::SystemTrayDelegate(QObject *parent):QStyledItemDelegate(parent) {

}

void SystemTrayDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing);
    QRect rect = option.rect;

//    if (option.state & QStyle::State_MouseOver)
//
//    else
//        painter->fillRect(rect, st::StyleDefine::instance().getSearchNormalColor());

    painter->fillRect(rect, QColor(255, 255, 255));
    auto name = index.data(EM_SYSTEM_TRAY_NAME).toString();
    auto count = index.data(EM_SYSTEM_TRAY_UNREAD_COUNT).toInt();

    // icon
    {
        auto dpi = st::qimage::dpi();
        auto pixmap = st::qimage::loadImage(":/QTalk/image1/newMessage.png", true, true, 24 * dpi);
        QRect imageRect(12 + rect.x(), (rect.height() - 24) / 2 + rect.y(), 24, 24);
        painter->drawPixmap(imageRect, pixmap);
    }
    // text
    {
        auto x = 12 + 10 + 24 + rect.x();
        QRect textRect(x, rect.y(), rect.width() - 12 - 30 - 30, rect.height());
        QTextOption textOption;
        textOption.setAlignment(Qt::AlignVCenter);

        painter->setPen(QColor(51,51,51));
        painter->drawText(textRect, name, textOption);
    }
    // count
    {
        auto x = rect.right() - 12 - 30;
        QRect textRect(x, rect.y(), 30, rect.height());
        QTextOption textOption;
        textOption.setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        painter->setPen(QColor(0,202,190));
        painter->drawText(textRect, QString::number(count), textOption);
    }

    painter->restore();
}

QSize SystemTrayDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
    const QSize& size = QStyledItemDelegate::sizeHint(option, index);
    return {size.width(), 48};
}

bool SystemTrayDelegate::editorEvent(QEvent *e, QAbstractItemModel *model, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) {
    if (e->type() == QEvent::MouseButtonPress)
    {
        auto chatType = index.data(EM_SYSTEM_TRAY_CHAT_TYPE).toInt();
        auto userId =index.data(EM_SYSTEM_TRAY_USER_ID).toString();
        auto realJid =index.data(EM_SYSTEM_TRAY_REAL_JID).toString();
        auto name =index.data(EM_SYSTEM_TRAY_NAME).toString();

        StSessionInfo sessionInfo(chatType, userId, name);
        sessionInfo.realJid = realJid;
        emit sgJumtoSession(sessionInfo);
    }
    return QStyledItemDelegate::editorEvent(e, model, option, index);
}

/** **/
SystemTrayPopWnd::SystemTrayPopWnd(QWidget *parent)
    :UShadowDialog(nullptr, true)
{
    Qt::WindowFlags flags = Qt::WindowDoesNotAcceptFocus | Qt::WindowContextHelpButtonHint | Qt::FramelessWindowHint
        | Qt::WindowFullscreenButtonHint | Qt::WindowCloseButtonHint | Qt::WindowTitleHint |
        Qt::WindowStaysOnTopHint;

#ifdef _WINDOWS
    flags |= Qt::Tool;
#else
    flags |= Qt::Widget;
#endif

    setWindowFlags(flags);
    // 设置不抢主窗口焦点
    setAttribute(Qt::WA_X11DoNotAcceptFocus, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    // title
    auto* titleFrm = new QFrame(this);
    titleFrm->setObjectName("SystemTrayTitleFrm");
    auto* iconLabel = new ImageLabel(":/QTalk/image1/logo.png", this);
    iconLabel->setFixedHeight(30);
    cancelAlert = new QLabel(tr("取消闪动"), this);
    cancelAlert->installEventFilter(this);
    cancelAlert->setObjectName("cancelAlert");
    auto* titleLay = new QHBoxLayout(titleFrm);
    titleLay->setContentsMargins(10, 0, 20, 0);
    titleLay->addWidget(iconLabel);
    titleLay->addWidget(cancelAlert);
    cancelAlert->setVisible(false);
    titleLay->setAlignment(cancelAlert, Qt::AlignRight | Qt::AlignVCenter);
    titleFrm->setFixedHeight(40);
    // list
    _pStackedWidget = new QStackedWidget(this);
    _pNewMessageView = new QListView(this);

    _pNewMessageView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _pNewMessageView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _pNewMessageView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _pNewMessageView->verticalScrollBar()->setSingleStep(10);
    _pNewMessageView->setFrameShape(QFrame::NoFrame);
    _pNewMessageView->setAutoScroll(false);
    auto* delegate = new SystemTrayDelegate(this);
    _pNewMessageView->setItemDelegate(delegate);

    _pNewMessageModel = new QStandardItemModel(this);
    _pNewMessageView->setModel(_pNewMessageModel);
    _pStackedWidget->addWidget(_pNewMessageView);

    // emptyLabel
    emptyLabel = new QLabel(tr("暂时没有未读消息"), this);
    emptyLabel->setObjectName("SystemTrayEmptyLabel");
    emptyLabel->setAlignment(Qt::AlignCenter);
    _pStackedWidget->addWidget(emptyLabel);
    _pStackedWidget->setCurrentWidget(emptyLabel);
    _pStackedWidget->setFixedHeight(74);
    // bottom
    //auto* bottomFrm = new QFrame(this);
    //bottomFrm->setObjectName("SystemTrayBottomFrm");
    //bottomFrm->setFixedHeight(48);
    //auto* bottomLay = new QHBoxLayout(bottomFrm);
    //bottomLay->setMargin(0);
    //auto* feedback = new ImageTextLabel(":/QTalk/image1/feedback.png", tr("反馈"));
    //auto* quit = new ImageTextLabel(":/QTalk/image1/quit.png", tr("退出"));
    //bottomLay->addWidget(feedback);
    //auto* line = new Line(Qt::Vertical, this);
    //line->setObjectName("wLine");
    //bottomLay->addWidget(line);
    //bottomLay->addWidget(quit);
    //
    auto* mainFrm = new QFrame(this);
    auto* bodyLay = new QVBoxLayout(mainFrm);
    bodyLay->setMargin(0);
    bodyLay->setSpacing(0);
    bodyLay->addWidget(titleFrm, 0);
    bodyLay->addWidget(_pStackedWidget, 0);
    this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
//    bodyLay->addWidget(bottomFrm);

    mainFrm->setObjectName("SystemTrayMainFrm");
    auto* mainLay = new QHBoxLayout(_pCenternWgt);
    mainLay->setMargin(0);
    mainLay->addWidget(mainFrm);

    qRegisterMetaType<StSessionInfo>("StSessionInfo");
 //   connect(feedback, &ImageTextLabel::clicked, this, &SystemTrayPopWnd::sgFeedback);
 //   connect(quit, &ImageTextLabel::clicked, this, &SystemTrayPopWnd::sgQuit);
    connect(this, &SystemTrayPopWnd::sgCancelAlert, this, &SystemTrayPopWnd::onClearNewMessage);
    connect(delegate, &SystemTrayDelegate::sgJumtoSession, this, &SystemTrayPopWnd::sgJumtoSession, Qt::QueuedConnection);
    connect(delegate, &SystemTrayDelegate::sgJumtoSession, this, &SystemTrayPopWnd::onClicked, Qt::QueuedConnection);
}

void SystemTrayPopWnd::setCancelAlertBtnVisible(bool show) {
    cancelAlert->setVisible(show);
}

bool SystemTrayPopWnd::hasNewMessage() {
    return !_data.empty();
}

bool SystemTrayPopWnd::eventFilter(QObject* o, QEvent* e)
{
    if(o == cancelAlert)
    {
        if(e->type() == QEvent::MouseButtonPress)
        {
            emit sgCancelAlert();
            return true;
        }
    }
    return UShadowDialog::eventFilter(o ,e);
}

//
void SystemTrayPopWnd::onNewMessage(int chatType, const st::entity::UID& uid, const QString& name, qint64 lastUpdateTime, int unread)
{
    //
    auto sortData = [](const StNewMessageData& a, const StNewMessageData& b){
        return a.lastUpdateTime > b.lastUpdateTime;
    };

    auto addItem = [this](const StNewMessageData& data, bool top){
        auto* item = new QStandardItem;
        item->setData(data._chatType, EM_SYSTEM_TRAY_CHAT_TYPE);
        item->setData(data.uid.qUsrId(), EM_SYSTEM_TRAY_USER_ID);
        item->setData(data.uid.qReadJid(), EM_SYSTEM_TRAY_REAL_JID);
        item->setData(data.name, EM_SYSTEM_TRAY_NAME);
        item->setData(data.unreadCount, EM_SYSTEM_TRAY_UNREAD_COUNT);
        item->setData(data.lastUpdateTime, EM_SYSTEM_TRAY_LAST_TIME);

        if(top)
            _pNewMessageModel->insertRow(0, item);
        else
            _pNewMessageModel->appendRow(item);
        _pNewMessageModel->sort(0);
        _pStackedWidget->setCurrentWidget(_pNewMessageView);
        _items[data.uid] = item;
    };

    auto removeItem = [this, sortData, addItem](const st::entity::UID &uid){
        auto itemFind = _items.find(uid);
        if(itemFind != _items.end())
        {
            _pNewMessageModel->removeRow((itemFind->second->row()));
            _items.erase(itemFind);
            _pNewMessageModel->sort(0);
        }
        //
        if(_data.empty())
        {
            emit sgCancelAlert();
        }
        else
        {
            std::sort(_data.begin(), _data.end(), sortData);
            for(const auto& data : _data )
            {
                if(_items.find(data.uid) != _items.end())
                    continue;
                //
                addItem(data, false);
                break;
            }
        }
    };

    auto resize = [this](){
        if(_items.empty())
        {
            _pStackedWidget->setCurrentWidget(emptyLabel);
            _pStackedWidget->setFixedHeight(74);
        }
        else
        {
            _pStackedWidget->setFixedHeight(_items.size() * 48);
            this->setFixedHeight(_items.size() * 48 + 40);
        }
    };

    // clear old data
    auto itFind = std::find_if(_data.begin(), _data.end(), [uid](const auto& d){
        return d.uid == uid;
    });

    if(itFind != _data.end())
        _data.erase(itFind);
    // if unread == 0 delete item
    if(unread == 0)
    {
        removeItem(uid);
        resize();
        return;
    }
    // add new data
    StNewMessageData data(chatType, uid, name, unread, lastUpdateTime);
    _data.push_back(data);
    // had item -> update data
    // else -> add new item && start alert
    if(_items.find(uid) == _items.end())
    {
        addItem(data, true);
        //
        if(_items.size() > 4)
        {
            st::entity::UID delUid;
            qint64 minTime = LLONG_MAX;
            for (const auto& it : _items)
            {
                qint64 tempT = it.second->data(EM_SYSTEM_TRAY_LAST_TIME).toLongLong();
                auto userId = it.second->data(EM_SYSTEM_TRAY_USER_ID).toString();
                auto realJid = it.second->data(EM_SYSTEM_TRAY_REAL_JID).toString();
                minTime = qMin(tempT, minTime);
                if(minTime == tempT)
                    delUid = st::entity::UID(userId, realJid);
            }

            auto delItem = _items[delUid];
            _pNewMessageModel->removeRow(delItem->row());
            _items.erase(delUid);
        }

        emit sgStartTimer();
        setCancelAlertBtnVisible(true);
    }
    else
    {
        auto* item = _items[uid];
        item->setData(name, EM_SYSTEM_TRAY_NAME);
        item->setData(unread, EM_SYSTEM_TRAY_UNREAD_COUNT);
    }

    resize();
}

//
void SystemTrayPopWnd::onClearNewMessage() {
    _pNewMessageModel->clear();
    _data.clear();
    _items.clear();
    _pStackedWidget->setCurrentWidget(emptyLabel);
    _pStackedWidget->setFixedHeight(74);
}

void SystemTrayPopWnd::onClicked(const StSessionInfo&) {
    this->setVisible(false);
}