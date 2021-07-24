//
// Created by cc on 2019/12/27.
//

#ifndef QTALK_V2_CHATMAINSOREMODEL_H
#define QTALK_V2_CHATMAINSOREMODEL_H

#include <QSortFilterProxyModel>
#include "ChatMainDelegate.h"

class ChatMainSoreModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit ChatMainSoreModel(QObject *parent = nullptr);

protected:
    bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};



#endif //QTALK_V2_CHATMAINSOREMODEL_H
