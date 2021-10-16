//
// Created by cc on 2019/11/01.
//

#ifndef STALK_V2_MESSAGERECORDMANAGER_H
#define STALK_V2_MESSAGERECORDMANAGER_H

#include <QFrame>
#include "SearchItemWgt.h"
#include <QListWidget>
#include <QListWidgetItem>
#include <QStandardItemModel>
#include <QStandardItem>
#include <vector>
#include <QStackedWidget>
#include <QSortFilterProxyModel>

#include "MessageSortModel.h"
#include "SearchUserView.h"
#include "CustomUi/UShadowWnd.h"
#include "Util/lazy/lazyq.h"
#include "include/CommonStrcut.h"

/**
* @description: SearchMessageManager
* @author: cc
**/
class Search_Edit;
class MessageRecordManager : public UShadowDialog
{
    Q_OBJECT
public:
    explicit MessageRecordManager(QWidget *parent = nullptr);
    ~MessageRecordManager() override ;

public:
    void setSearch(const QString &key, const QString &xmppId);

protected:
    void changeEvent(QEvent *event) override ;

private:
    void initUI();

private:
    void searchMessageList(const QString &key);
    void goSearchList();
    void updateSearchList();

    void goSearchUser();
    void getUserMessage(const QString &, const QString &, int);
    void updateSearchUser();

    void showMessageDetail(qint64, const QString &, int);
    //
    void getMoreMessage(bool up);
    void messageRequest(const QInt64 &time,
                        int chatType,
                        const QString &xmppId,
                        bool up);

private slots:
    void onSearchUserScroll(int);
    void onMessageWndScroll(int);
    void updateMessageWnd(bool up);

Q_SIGNALS:
    void sgGoSearch(const QString &);
    void sgUpdateSearchList();
    void sgUpdateUserSearch();
    void sgGotMessage(bool up);

private:
    std::string _select_user{};

    QListView *_pSearchView{};
    QStandardItemModel *_pSearchModel{};

    QListView *_pSearchUserView{};
    QStandardItemModel *_pSearchUserModel{};

    QFrame   *messageFrm{};
    QListView *_pMessageView{};
    QStandardItemModel *_pMessageModel{};
    MessageSortModel   *_pSortModel{};

    Search_Edit *_pSearchEdit{};

    QLabel     *_pLoadingContent{};
    QLabel     *_pLoadingMoreL{};
    QLabel     *_pLoadingMoreR_T{};
    QLabel     *_pLoadingMoreR_B{};
    QStackedWidget *_pStackWgt{};

private:
    lazyq<QString> *_searchQueue{nullptr};

private:
    bool    _listHasMore {false};
    bool    _userHasMore = {false};
    bool    _messageHasMore_b = {true}; // before
    bool    _messageHasMore_a = {true}; // after

    std::vector<st::Search::StHistory> _search_history_list; // 关键字结果
    std::vector<StNetMessageResult> _search_user_search;  // 关键字 + 人 结果
    std::vector<StNetMessageResult> _messages;  // all-message

private:
    std::string _key{};
    int         _list_pos{};

    int         _user_pos{};
    int         _user_type{};
    int         _user_scroll_value{};
    int         _message_scroll_value{};
    QString     _user_id;
};


#endif //STALK_V2_MESSAGERECORDMANAGER_H
