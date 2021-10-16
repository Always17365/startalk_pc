//
// Created by cc on 2019-07-08.
//
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#ifndef STALK_V2_SEARCHVIEW_H
#define STALK_V2_SEARCHVIEW_H

#include <QFrame>
#include <QListView>
#include <QStandardItemModel>
#include "entity/UIEntity.h"
#include "SearchItemDelegate.h"


/**
* @description: SearchView
* @author: cc
* @create: 2019-07-08 16:33
**/
class SearchView : public QListView{
    Q_OBJECT
public:
    explicit SearchView(QWidget* parent = nullptr);
    ~SearchView() override;

Q_SIGNALS:
    void sgOpenNewSession(const StSessionInfo &into);
    void sgGetMore(int );
    void sgSwitchFun(int );
    void sgShowMessageRecordWnd(const QString&, const QString&);
    void sgShowFileRecordWnd(const QString&);

public:
    void selectUp();
    void selectDown();
    void selectItem();
    void removeMoreBar();
    void addOpenWithIdItem(const QString& keyId);

public:
    void addSearchResult(const st::Search::StSearchResult& ret, int reqType, bool isGetMore);
    void onHoverIndexChanged(const QModelIndex&);

public:
    void clear();

private:
    void onOpenNewSession(int type, const QString &xmppid, const QString &name, const QString &icon);

private:
    QStandardItemModel *_srcModel;
    SearchItemDelegate* _pItemDelegate;
};


#endif //STALK_V2_SEARCHVIEW_H
