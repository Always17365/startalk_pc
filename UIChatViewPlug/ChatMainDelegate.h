//
// Created by cc on 2019/12/27.
//

#ifndef STALK_V2_CHATMAINDELEGATE_H
#define STALK_V2_CHATMAINDELEGATE_H

#include <QStyledItemDelegate>
#include <QListView>


enum {
    EM_USER_TYPE = Qt::UserRole + 1,
    EM_USER_MSG_TYPE,
    EM_USER_MSG_TIME,
    EM_USER_INFO,
    EM_USER_ITEM_TYPE,
    EM_USER_SHOW_SHARE
};

class MessageItemBase;
class ChatMainDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ChatMainDelegate(QMap<QString, MessageItemBase*> &mapItemWgt, QListView* parent);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

public:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

public:
    void dealWidget(const QModelIndex &index);
    QWidget* creatWgt(const QModelIndex &index);

Q_SIGNALS:
    void sgGetMessageDetail(qint64, const QString&, int);

private:
    QListView* _parentWgt = nullptr;

    QMap<QString, MessageItemBase*> &_mapItems;
};

#endif //STALK_V2_CHATMAINDELEGATE_H
