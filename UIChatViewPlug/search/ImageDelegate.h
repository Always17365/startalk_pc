//
// Created by cc on 2019/12/02.
//

#ifndef STALK_V2_IMAGEDELEGATE_H
#define STALK_V2_IMAGEDELEGATE_H

#include <QStyledItemDelegate>
#include <QTableView>

struct StImageResult {
    QString     msg_id;
    qint64      time;
    QString     from;
    QString     to;
    QString     content;
    QString     imageLink;
    int         index;
};
Q_DECLARE_METATYPE(StImageResult)

/**
* @description: ImageDelegate
* @author: cc
* @create: 2019-12-02 14:55
**/

class ImageDelegate  : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit ImageDelegate(QTableView* parent);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    void dealWidget(const QStyleOptionViewItem &option, const QModelIndex &index);
    QWidget* creatWgt(const QStyleOptionViewItem &option, const QModelIndex &index);

private:
    QTableView* _parentWgt = nullptr;
};

#endif //STALK_V2_IMAGEDELEGATE_H
