//
// Created by cc on 2019/11/01.
//

#include "MessageRecordManager.h"
#include <QVBoxLayout>
#include <QSplitter>
#include <QScrollBar>
#include <QDebug>
#include <QMovie>
#include <QtConcurrent>
#include "../../CustomUi/TitleBar.h"
#include "../../CustomUi/SearchEdit.hpp"
#include "../ChatViewMainPanel.h"
#include "../MessageManager.h"
#include "SearchListDelegate.h"
#include "../../Platform/Platform.h"
#include "MessageAnalysis.h"
#include "MessageDelegate.h"
#include "../../Platform/dbPlatForm.h"
#include "../../UICom/uicom.h"
#include "../ChatUtil.h"

extern ChatViewMainPanel *g_pMainPanel;
/** MessageRecordManager **/
MessageRecordManager::MessageRecordManager(QWidget *parent)
    : UShadowDialog(parent, true)
{
//    setWindowFlags(this->windowFlags() | Qt::WindowStaysOnTopHint);
    initUI();
    setMinimumSize(800, 720);
    std::function<int(STLazyQueue<QString>*)> searchFun = [this](STLazyQueue<QString> *q) ->int
    {
        int runningCount = 0;

        if (!q->empty())
        {
            QString key = q->tail();

            while (!q->empty())
            {
                q->pop();
                runningCount++;
            }

            emit sgGoSearch(key);
        }

        return runningCount;
    };
    _searchQueue = new STLazyQueue<QString>(300, searchFun);
}

MessageRecordManager::~MessageRecordManager() = default;

void MessageRecordManager::initUI()
{
#ifdef _MACOS
    QWidget *wgt = UICom::getInstance()->getAcltiveMainWnd();

    if(wgt && wgt->isFullScreen())
        this->setWindowFlags(this->windowFlags() | Qt::Tool);

#endif
//
    auto *titleBar = new TitleBar(tr("消息管理器"), this, this);
    titleBar->setFixedHeight(50);
    setMoverAble(true, titleBar);
    //
    auto *searchLay = new QHBoxLayout;
    {
        searchLay->setMargin(16);
        _pSearchEdit = new Search_Edit(this);
        _pSearchEdit->setFixedHeight(30);
        searchLay->addWidget(_pSearchEdit);
    }
    _pSearchView = new QListView(this);
    _pSearchView->setFrameShape(QFrame::NoFrame);
    _pSearchModel = new QStandardItemModel(this);
    _pSearchView->setModel(_pSearchModel);
    _pSearchView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _pSearchView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _pSearchView->verticalScrollBar()->setSingleStep(12);
    auto *itemDelegate = new SearchListDelegate(this);
    _pSearchView->setItemDelegate(itemDelegate);
    auto *leftFrm = new QFrame(this);
    auto *leftLay = new QVBoxLayout(leftFrm);
    leftLay->setMargin(0);
    leftLay->setSpacing(0);
    leftLay->addLayout(searchLay);
    leftLay->addWidget(_pSearchView);
    _pLoadingMoreL = QTalk::makeLoadingLabel(true, {50, 50}, this);
    _pLoadingMoreL->setVisible(false);
    leftLay->addWidget(_pLoadingMoreL);
    _pLoadingContent = QTalk::makeLoadingLabel(false, {0, 0}, this);
    _pLoadingMoreR_T = QTalk::makeLoadingLabel(true, {50, 50}, this);
    _pLoadingMoreR_B = QTalk::makeLoadingLabel(true, {50, 50}, this);
    _pLoadingMoreR_T->setVisible(false);
    _pLoadingMoreR_B->setVisible(false);
    _pStackWgt = new QStackedWidget(this);
    _pStackWgt->addWidget(_pLoadingContent);
    //
    _pSearchUserView = new SearchUserView(this);
    _pSearchUserView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _pSearchUserView->verticalScrollBar()->setSingleStep(12);
    _pStackWgt->addWidget(_pSearchUserView);
    _pStackWgt->setCurrentWidget(_pSearchUserView);
    _pSearchUserModel = new QStandardItemModel(this);
    _pSearchUserView->setModel(_pSearchUserModel);
    auto *searchUserDelegate = new MessageDelegate(_pSearchUserView);
    _pSearchUserView->setItemDelegate(searchUserDelegate);
    //
    messageFrm = new QFrame(this);
    auto *messageLay = new QVBoxLayout(messageFrm);
    messageLay->setMargin(0);
    messageLay->setSpacing(16);
    auto *backBtn = new QPushButton(tr("< 返回"), this);
    backBtn->setObjectName("SearchBackButton");
    messageLay->addWidget(backBtn);
    messageLay->addWidget(_pLoadingMoreR_T);
    messageLay->setAlignment(backBtn, Qt::AlignLeft);
    _pMessageView = new QListView(this);
    messageLay->addWidget(_pMessageView);
    _pMessageView->setFrameShape(QFrame::NoFrame);
    _pMessageView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _pMessageView->verticalScrollBar()->setSingleStep(12);
    _pMessageView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _pMessageView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _pStackWgt->addWidget(messageFrm);
    _pMessageModel = new QStandardItemModel(this);
    _pSortModel = new MessageSortModel(this);
    _pSortModel->setSourceModel(_pMessageModel);
    _pSortModel->setDynamicSortFilter(false);
    _pMessageView->setModel(_pSortModel);
    auto *messageDelegate = new MessageDelegate(_pMessageView);
    _pMessageView->setItemDelegate(messageDelegate);
    //
    auto *rightFrm = new QFrame(this);
    auto *rightLay = new QVBoxLayout(rightFrm);
    rightLay->setContentsMargins(0, 8, 0, 8);
    rightLay->setSpacing(0);
//    rightLay->addWidget(_pLoadingMoreR_T);
    rightLay->addWidget(_pStackWgt);
    rightLay->addWidget(_pLoadingMoreR_B);
    auto *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);
    splitter->addWidget(leftFrm);
    splitter->addWidget(rightFrm);
    splitter->setStretchFactor(1, 1);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
    //
    auto *mainFrm = new QFrame(this);
    mainFrm->setObjectName("MainFrm");
    auto *mainLay = new QVBoxLayout(mainFrm);
    mainLay->setContentsMargins(0, 0, 0, 6);
    mainLay->setSpacing(0);
    mainLay->addWidget(splitter);
    mainLay->setStretch(0, 0);
    mainLay->setStretch(1, 1);
    auto *layout = new QVBoxLayout(_pCenternWgt);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(titleBar);
    layout->addWidget(mainFrm);
    connect(_pSearchEdit, &Search_Edit::textChanged, this, [this](const QString & text)
    {
        _searchQueue->push(text);
    });
    connect(this, &MessageRecordManager::sgGoSearch, this, &MessageRecordManager::searchMessageList);
    connect(this, &MessageRecordManager::sgUpdateSearchList, this, &MessageRecordManager::updateSearchList);
    connect(this, &MessageRecordManager::sgUpdateUserSearch, this, &MessageRecordManager::updateSearchUser);
    connect(this, &MessageRecordManager::sgGotMessage, this, &MessageRecordManager::updateMessageWnd);
    connect(itemDelegate, &SearchListDelegate::sgGetUserMessages, this, &MessageRecordManager::getUserMessage);
    connect(itemDelegate, &SearchListDelegate::sgGetMore, this, [this]()
    {
        _pLoadingMoreL->movie()->start();
        _pLoadingMoreL->setVisible(true);
        goSearchList();
    });
    connect(searchUserDelegate, &MessageDelegate::sgGetMessageDetail, this, &MessageRecordManager::showMessageDetail);
    connect(_pSearchUserView->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &MessageRecordManager::onSearchUserScroll);
    // all message window scroll event
    connect(_pMessageView->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &MessageRecordManager::onMessageWndScroll);
    connect(backBtn, &QPushButton::clicked, this,  [this]()
    {
        _pStackWgt->setCurrentWidget(_pSearchUserView);
    });
}

void MessageRecordManager::setSearch(const QString &key, const QString &xmppId)
{
    if (_pSearchEdit)
    {
        _select_user = xmppId.toStdString();

        if(_pSearchEdit->text() != key)
            _pSearchEdit->setText(key);
        else
            searchMessageList(key);
    }
}

//
void MessageRecordManager::goSearchList()
{
    using namespace QTalk::Search;
    _listHasMore = false;
    _search_history_list.clear();

    if (g_pMainPanel)
    {
        QtConcurrent::run([this]()
        {
            SearchInfoEvent searchInfo;
            searchInfo.start = _list_pos;
            searchInfo.length = 15;
            _list_pos += 15;
            searchInfo.key = _key;
            searchInfo.action = EM_ACTION_HS_SINGLE | EM_ACTION_HS_MUC;
            ChatMsgManager::sendSearch(searchInfo);
            _search_history_list.clear();

            for(const auto &it : searchInfo.searchRet)
            {
                const auto history = it.second._history;
                _search_history_list.insert(_search_history_list.end(), history.begin(), history.end());
                _listHasMore |= it.second.hasMore;
            }

            emit sgUpdateSearchList();
        });
    }
}

void MessageRecordManager::searchMessageList(const QString &key)
{
    _listHasMore = false;
    _pSearchUserModel->clear();
    _pSearchModel->clear();
    _user_id = "";
    _list_pos = 0;
    _key = key.toStdString();
    _pStackWgt->setCurrentWidget(_pSearchUserView);
    //
    goSearchList();
}

//
void MessageRecordManager::updateSearchList()
{
    using namespace QTalk::Search;
    QStandardItem *selectItem = nullptr;

    for(const auto &info : _search_history_list)
    {
        auto *item = new QStandardItem;
        item->setData(EM_SEARCH_LIST_ROLE_ITEM, EM_SEARCH_LIST_DATA_ROLE);

        // todo 接口问题
//        item->setData(info.name.data(), EM_SEARCH_LIST_DATA_NAME);
        if(EM_ACTION_HS_SINGLE == info.type && info.from != info.to)
        {
            std::string selfXmppId = PLAT.getSelfXmppId();
            std::string id = selfXmppId == info.from ? info.to : info.from;

            if(!_select_user.empty() && id == _select_user)
                selectItem = item;

            item->setData(QTalk::getUserNameNoMask(id).data(), EM_SEARCH_LIST_DATA_NAME);
            auto user_info = DB_PLAT.getUserInfo(id);

            if(user_info)
                item->setData(QTalk::GetHeadPathByUrl(user_info->HeaderSrc).data(), EM_SEARCH_LIST_DATA_HEAD);
        }
        else
        {
            if(!_select_user.empty() && info.to == _select_user)
                selectItem = item;

            item->setData(info.name.data(), EM_SEARCH_LIST_DATA_NAME);
            item->setData(QTalk::GetHeadPathByUrl(info.icon).data(), EM_SEARCH_LIST_DATA_HEAD);
        }

        item->setData(info.type, EM_SEARCH_LIST_DATA_TO_TYPE);
        item->setData(info.key.data(), EM_SEARCH_LIST_DATA_KEY);

        if(info.type == QTalk::Search::EM_ACTION_HS_MUC || info.to == info.from || PLAT.getSelfXmppId() == info.from)
            item->setData(info.to.data(), EM_SEARCH_LIST_DATA_XMPP_ID);
        else
            item->setData(info.from.data(), EM_SEARCH_LIST_DATA_XMPP_ID);

        if(info.count <= 1)
            item->setData(info.body.data(), EM_SEARCH_LIST_DATA_SUB_MESSAGE);
        else
        {
            QString content = QObject::tr("%1条与“%2”相关聊天记录").arg(info.count).arg(info.key.data());
            item->setData(content, EM_SEARCH_LIST_DATA_SUB_MESSAGE);
        }

        _pSearchModel->appendRow(item);
    }

    //
    if(nullptr != selectItem)
    {
        _pSearchView->setCurrentIndex(selectItem->index());
        auto type = selectItem->data(EM_SEARCH_LIST_DATA_TO_TYPE).toInt();
        getUserMessage(_key.data(), _select_user.data(), type);
        _select_user = {};
    }

    if(_pLoadingMoreL->isVisible())
    {
        _pLoadingMoreL->movie()->stop();
        _pLoadingMoreL->setVisible(false);
    }

    //
    if (_listHasMore)
    {
        auto *item = new QStandardItem;
        item->setData(EM_SEARCH_LIST_ROLE_GET_MORE, EM_SEARCH_LIST_DATA_ROLE);
        _pSearchModel->appendRow(item);
    }
}

//
void MessageRecordManager::goSearchUser()
{
    using namespace QTalk::Search;
    _userHasMore = false;
    _search_user_search.clear();

    if (g_pMainPanel)
    {
        QtConcurrent::run([this]()
        {
            SearchInfoEvent searchInfo;
            searchInfo.start = _user_pos;
            searchInfo.length = 20;
            _user_pos += 20;
            searchInfo.key = _key;
            searchInfo.action = _user_type;

            if (EM_ACTION_HS_SINGLE == _user_type)
                searchInfo.to_user = _user_id.toStdString();
            else if(EM_ACTION_HS_MUC == _user_type)
                searchInfo.to_muc = _user_id.toStdString();

            ChatMsgManager::sendSearch(searchInfo);

            for(const auto &it : searchInfo.searchRet)
            {
                const auto history = it.second._history;
                _userHasMore |= it.second.hasMore;

                for(const auto &his : history)
                {
                    StNetMessageResult info;
                    info.msg_id = his.msg_id.data();
                    info.msg_type = his.msg_type;

                    if (EM_ACTION_HS_SINGLE == _user_type)
                        info.type = QTalk::Enum::TwoPersonChat;
                    else if(EM_ACTION_HS_MUC == _user_type)
                        info.type = QTalk::Enum::GroupChat;

                    info.time = his.time;
                    info.from = his.from.data();
                    info.to = his.to.data();
                    info.body = his.body.data();
                    info.extend_info = his.extend_info.data();
                    info.xmpp_id = _user_id;
                    QTalk::analysisMessage(info);
                    _search_user_search.push_back(info);
                }
            }

            emit sgUpdateUserSearch();
        });
    }
}

//
void MessageRecordManager::messageRequest(const QInt64 &time, int type, const QString &xmppId, bool up)
{
    // clear data
    _messages.clear();
    QtConcurrent::run([this, time, xmppId, type, up]()
    {
        auto messages = ChatMsgManager::getNetHistoryMessage(
                            time, type, QTalk::Entity::UID(xmppId), !up);

        for(const auto &message : messages)
        {
            StNetMessageResult info;
            info.msg_id = message.MsgId.data();
            info.msg_type = message.Type;
            info.type = message.ChatType ;
            info.time = message.LastUpdateTime;
            info.from = message.From.data();
            info.body = message.Content.data();
            info.extend_info = message.ExtendedInfo.data();
            info.xmpp_id = message.XmppId.data();
            QTalk::analysisMessage(info);
            _messages.push_back(info);
        }

        //
        if(messages.empty())
        {
            if (up)
                _messageHasMore_b = false;
            else
                _messageHasMore_a = false;
        }

        emit sgGotMessage(up);
    });
}

//
void MessageRecordManager::showMessageDetail(qint64 time, const QString &xmppId, int type)
{
    _pLoadingContent->movie()->start();
    _pStackWgt->setCurrentWidget(_pLoadingContent);
    _messageHasMore_b = true;
    _messageHasMore_a = true;
    // clear old data
    _pMessageModel->clear();
    // request
    messageRequest(time - 1, type, xmppId, false);
}

// get user/group history messages
void MessageRecordManager::getUserMessage(const QString &key, const QString &id, int type)
{
    // 避免重复点击
    if (_user_id == id)
        return;

    // clear data
    _user_pos = 0;
    _user_scroll_value = 0;
    _user_id = id;
    _user_type = type;
    // show loading movie
    _pLoadingContent->movie()->start();
    _pStackWgt->setCurrentWidget(_pLoadingContent);
    // clear model
    _pSearchUserModel->clear();
    // request
    goSearchUser();
}

// update user/group search result
void MessageRecordManager::updateSearchUser()
{
    // update data
    for(const auto &info : _search_user_search)
    {
        auto *item = new QStandardItem();
        item->setData(info.type, SEARCH_USER_TYPE);
        item->setData(info.msg_type, SEARCH_USER_MSG_TYPE);
        item->setData(QVariant::fromValue(info), SEARCH_USER_INFO);
        _pSearchUserModel->appendRow(item);
    }

    // deal loading movie
    if(_pStackWgt->currentWidget() == _pLoadingContent)
    {
        _pLoadingContent->movie()->stop();
        _pStackWgt->setCurrentWidget(_pSearchUserView);
    }

    if(_pLoadingMoreR_B->isVisible())
    {
        _pLoadingMoreR_B->movie()->stop();
        _pLoadingMoreR_B->setVisible(false);
    }
}

// user/group search result window scroll event
// get more search result
void MessageRecordManager::onSearchUserScroll(int value)
{
    // no more -> return
    if(!_userHasMore)
        return;

    // scroll bar direction
    if (value <= _user_scroll_value)
        return;

    //
    _user_scroll_value = value;
    auto max = _pSearchUserView->verticalScrollBar()->maximum();

    if(max - value <= 5)
    {
        // less then 500 ms -> return
        static qint64 req_time = 0;
        auto now = QDateTime::currentMSecsSinceEpoch();

        if(now - req_time < 500)
            return;

        req_time = now;
        // show loading movie
        _pLoadingMoreR_B->movie()->start();
        _pLoadingMoreR_B->setVisible(true);
        // request
        goSearchUser();
    }
}

// all-message window scroll event
// get more message
void MessageRecordManager::onMessageWndScroll(int value)
{
    // max
    auto max = _pMessageView->verticalScrollBar()->maximum();
    // scroll bar direction
    bool up = value <= _message_scroll_value;

    if (up)
    {
        if(!_messageHasMore_b)
            return;

        if(value > 10)
            return;
    }
    else
    {
        if(!_messageHasMore_a)
            return;

        if(max - value > 10)
            return;
    }

    //
    _message_scroll_value = value;
    // less then 500 ms -> return
    static qint64 req_time = 0;
    auto now = QDateTime::currentMSecsSinceEpoch();

    if(now - req_time < 500)
        return;

    req_time = now;

    if (up)
    {
        // show loading movie
        _pLoadingMoreR_T->movie()->start();
        _pLoadingMoreR_T->setVisible(true);
    }
    else
    {
        // show loading movie
        _pLoadingMoreR_B->movie()->start();
        _pLoadingMoreR_B->setVisible(true);
    }

    // request
    getMoreMessage(up);
}

void MessageRecordManager::getMoreMessage(bool up)
{
    QModelIndex index = up ? _pSortModel->index(0, 0) : _pSortModel->index(_pSortModel->rowCount() - 1, 0);

    if(!index.isValid())
        return;

    StNetMessageResult data = index.data(SEARCH_USER_INFO).value<StNetMessageResult>();

    if(up)
    {
        _pLoadingMoreR_T->movie()->start();
        _pLoadingMoreR_T->setVisible(true);
    }
    else
    {
        _pLoadingMoreR_B->movie()->start();
        _pLoadingMoreR_B->setVisible(true);
    }

    // request
    messageRequest(data.time, data.type, data.xmpp_id, up);
}

// update and show all-message window
void MessageRecordManager::updateMessageWnd(bool up)
{
    // set new data
    for(const auto &info : _messages)
    {
        auto *item = new QStandardItem();
        item->setData(info.type, SEARCH_USER_TYPE);
        item->setData(info.msg_type, SEARCH_USER_MSG_TYPE);
        item->setData(info.time, SEARCH_USER_MSG_TIME);
        item->setData(QVariant::fromValue(info), SEARCH_USER_INFO);
        _pMessageModel->appendRow(item);
    }

    _pSortModel->sort(0);

    // show window
    if(_pStackWgt->currentWidget() == _pLoadingContent)
    {
        _pLoadingContent->movie()->stop();
        _pStackWgt->setCurrentWidget(messageFrm);
    }

    if(_pLoadingMoreR_T->isVisible())
    {
        _pLoadingMoreR_T->movie()->stop();
        _pLoadingMoreR_T->setVisible(false);
    }

    if(_pLoadingMoreR_B->isVisible())
    {
        _pLoadingMoreR_B->movie()->stop();
        _pLoadingMoreR_B->setVisible(false);
    }
}

void MessageRecordManager::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::WindowStateChange)
    {
#ifdef _MACOS
        auto sts = this->windowState();

        if((sts & Qt::WindowFullScreen))
            setWindowFlags(this->windowFlags() | Qt::Tool);
        else
            setWindowFlags(this->windowFlags() & ~Qt::Tool);

#endif
    }

    UShadowDialog::changeEvent(event);
}
