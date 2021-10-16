//
// Created by cc on 2019/11/14.
//

#ifndef STALK_V2_FILERECORDWND_H
#define STALK_V2_FILERECORDWND_H


#include <QListView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStackedWidget>
#include <vector>
#include <QStyledItemDelegate>

#include "CustomUi/UShadowWnd.h"
#include "CustomUi/SearchEdit.hpp"
#include "include/CommonStrcut.h"
#include "Util/lazy/lazyq.h"

/**
 *
 */
struct StFileRecord {

    QString msg_id;
    QString from_name;
    QString source;
    QString file_name;
    QString file_md5;
    QString file_size;
    QString file_link;

};

Q_DECLARE_METATYPE(StFileRecord)

enum {
    EM_FILE_RECORD_INFO = Qt::UserRole + 1
};

/**
* @description: FileRecordItemWgt
* @author: cc
**/
class FileRecordItemWgt : public QFrame
{
    Q_OBJECT
public:
    explicit FileRecordItemWgt(const StFileRecord &record, QWidget *parent = nullptr);

private:
    void onOpenFilePath();

private:
    std::string _msg_id;
    std::string _md5;
    QString     _file_link;
};

/**
* @description: FileRecordDelegate
* @author: cc
**/
class FileRecordDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit FileRecordDelegate(QListView *listView = nullptr);

protected:
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

private:
    void dealWidget(const QStyleOptionViewItem &option, const QModelIndex &index);
    QWidget *creatWgt(const QStyleOptionViewItem &option, const QModelIndex &index);

private:
    QListView *_pParentView{};
};


/**
* @description: FileRecordWnd
* @author: cc
* @create: 2019-11-14 18:07
**/
class FileRecordWnd : public UShadowDialog
{
    Q_OBJECT
public:
    explicit FileRecordWnd(QWidget *parent = nullptr);

public:
    void setSearch(const QString &key);

private:
    void initUI();
    void searchFile(const QString &key);
    void goSearch();

    void updateUi(const QString &);
    void onScrollChanged(int );

Q_SIGNALS:
    void sgGotResult(const QString &);
    void sgGoSearch(const QString &);

private:
    QStackedWidget *_pStackWgt{};

    QLabel     *_pLoading{};
    QLabel     *_pLoadingBottom{};

    Search_Edit *_pSearchEdit{};
    QListView   *_pView{};
    QStandardItemModel *_pModel{};

private:
    bool _hasMore{};
    std::string _key{};

    int _pos{};

    int _scroll_value;

private:
    std::vector<st::Search::StHistoryFile> _fileRecords;

private:
    lazyq<QString> *_searchQueue;
};


#endif //STALK_V2_FILERECORDWND_H
