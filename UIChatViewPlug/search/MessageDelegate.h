//
// Created by cc on 2019/11/05.
//

#ifndef STALK_V2_MESSAGEDELEGATE_H
#define STALK_V2_MESSAGEDELEGATE_H

#include <QStyledItemDelegate>
#include <QListView>

enum {
    SEARCH_USER_TYPE = Qt::UserRole + 1,
    SEARCH_USER_MSG_TYPE,
    SEARCH_USER_MSG_TIME,
    SEARCH_USER_INFO,
    SEARCH_USER_ITEM_TYPE
};

/**
* @description: SearchUserDelegate
* @author: cc
* @create: 2019-11-05 20:48
**/
class SearchItemBase;
class MessageDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit MessageDelegate(QListView* parent);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void dealWidget(const QStyleOptionViewItem &option, const QModelIndex &index);
    SearchItemBase* creatWgt(const QStyleOptionViewItem &option, const QModelIndex &index);

Q_SIGNALS:
    void sgGetMessageDetail(qint64, const QString&, int);

private:
    QListView* _parentWgt = nullptr;
};


#endif //STALK_V2_MESSAGEDELEGATE_H
