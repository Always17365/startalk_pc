//
// Created by cc on 2019-06-25.
//

#ifndef STALK_V2_LOCALSEARCHMAINWGT_H
#define STALK_V2_LOCALSEARCHMAINWGT_H

#include <QStackedWidget>
#include <QListView>
#include <QStandardItemModel>
#include <QListWidget>
#include <QCalendarWidget>
#include <QMutexLocker>
#include <QTableView>

#include "ItemWgt.h"
#include "SearchItemWgt.h"
#include "SearchUserView.h"
#include "MessageSortModel.h"
#include "SearchItemDelegate.h"
#include "entity/UID.h"
#include "entity/im_message.h"
#include "CustomUi/SearchEdit.hpp"
#include "CustomUi/UShadowWnd.h"
#include "Util/lazy/lazyq.h"

/**
* @description: SearchMainWgt
* @author: cc
* @create: 2019-06-25 11:02
**/
class TitleBar;
class LocalSearchMainWgt : public UShadowDialog
{
    Q_OBJECT
public:
    explicit LocalSearchMainWgt(QWidget *parent = nullptr);
    ~LocalSearchMainWgt() override ;

public:
    void showAllMessage(const st::entity::UID &uid);
    void resetUi();
    void initStyle(const QString &qss);

protected:
    bool eventFilter(QObject *o, QEvent *e) override ;
    void closeEvent(QCloseEvent *e) override ;
    void hideEvent(QHideEvent *e) override ;

private:
    void initUi();
    void initList();
    //
    bool dealMessages(const std::function<std::vector<st::entity::ImMessageInfo>()> &func,
                      QStandardItemModel *model, QSortFilterProxyModel *sortModel);
    void dealAllMessages(bool up, const std::function<std::vector<st::entity::ImMessageInfo>()> &func);
    void dealSearchMessages(const std::function<std::vector<st::entity::ImMessageInfo>()> &func);
    void dealFileMessages(const std::function<std::vector<st::entity::ImMessageInfo>()> &func);

Q_SIGNALS:
    void sgGetBeforeAllMessage(qint64 time);
    void sgGetBeforeFileMessage(qint64 time);
    void sgGetImageMessage(qint64 time);
    void sgGetLinkMessage(qint64 time);
    void sgSearch(qint64, const QString &);
    void sgPositionMessage(qint64, bool);
    void sgUpdateName(const QString &name);
    void sgInitStyle(const QString &qss);

private slots:
    void onButtonClicked(int id);
    void starSearch(qint64, const QString &);
    void onPositionMessage(qint64, bool);
    void onSetName(const QString &name);
    void onGetBeforeAllMessage(qint64 time);
    void onSearch(qint64 time, const QString &);
    void onGetBeforeFileMessage(qint64 time);
    void onGetImageMessage(qint64 time);
    void onGetLinkMessage(qint64 time);

public:
    enum {
        EM_INVALID_,
        EM_ALL,
        EM_IMG,
        EM_FILE,
        EM_LINK,
        EM_SEARCH
    };

private:
    TitleBar *_pTitleLabel{};
    Search_Edit *_searEdit{};
    QPushButton *_pCalendarBtn{};
    QCalendarWidget *_calendarWgt{};
    QStackedWidget *_pStackedWidget{};

    // all
    QListView          *_pAllListWgt{};
    QStandardItemModel *_pAllLstModel{};
    MessageSortModel   *_pAllSortModel{};

    // search
    SearchUserView     *_pSearchListWgt{};
    QStandardItemModel *_pSearchModel{};
    MessageSortModel   *_pSearchSortModel{};

    // file
    QListView          *_pFileListWgt{};
    QStandardItemModel *_pFileModel{};
    MessageSortModel   *_pFileSortModel{};

    // image
    QTableView         *_pImageView{};
    QStandardItemModel *_pImageModel{};

    // link
    QListView          *_pLinkListView{};
    QStandardItemModel *_pLinkModel{};
    MessageSortModel   *_pLinkSortModel{};

    QButtonGroup *_btnGroup{};
    QFrame       *_btnFrm{};
    QLabel       *_pSearchCountLabel{};

    lazyq<std::string> *_lazyQueue{nullptr};

private:
    QMutex             _mutex;
    QMap<int, bool >   _hasBefore;
    QMap<int, bool >   _hasNew;

    st::entity::UID _uid;

private:
    QLabel     *_pLoadingContent{};
    QLabel     *_pLoadingMoreR_T{};
    QLabel     *_pLoadingMoreR_B{};
};


#endif //STALK_V2_LOCALSEARCHMAINWGT_H
