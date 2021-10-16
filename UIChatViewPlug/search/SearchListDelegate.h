//
// Created by cc on 2019/11/05.
//

#ifndef STALK_V2_SEARCHLISTDELEGATE_H
#define STALK_V2_SEARCHLISTDELEGATE_H

#include <QStyledItemDelegate>

/**
* @description: SearchListDelegate
* @author: cc
* @create: 2019-11-05 19:06
**/

enum {
    EM_SEARCH_LIST_DATA_ROLE = Qt::UserRole + 1,
    EM_SEARCH_LIST_DATA_NAME,
    EM_SEARCH_LIST_DATA_HEAD,
    EM_SEARCH_LIST_DATA_XMPP_ID,
    EM_SEARCH_LIST_DATA_TO_TYPE,
    EM_SEARCH_LIST_DATA_SUB_MESSAGE,
    EM_SEARCH_LIST_DATA_KEY,
};

enum {
    EM_SEARCH_LIST_ROLE_ITEM,
    EM_SEARCH_LIST_ROLE_GET_MORE
};

class SearchListDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SearchListDelegate(QWidget * parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override;

Q_SIGNALS:
    void sgGetUserMessages(const QString&, const QString&, int);
    void sgGetMore();
};



#endif //STALK_V2_SEARCHLISTDELEGATE_H
