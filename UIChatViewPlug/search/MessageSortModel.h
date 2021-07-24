//
// Created by cc on 2019/11/28.
//

#ifndef QTALK_V2_MESSAGESORTMODEL_H
#define QTALK_V2_MESSAGESORTMODEL_H

#include <QSortFilterProxyModel>
#include "MessageDelegate.h"

/**
* @description: MessageSortModel
* @author: cc
**/
class MessageSortModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit MessageSortModel(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};

#endif //QTALK_V2_MESSAGESORTMODEL_H
