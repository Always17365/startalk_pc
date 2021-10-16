//
// Created by cc on 2019/11/29.
//

#ifndef STALK_V2_SEARCHUSERVIEW_H
#define STALK_V2_SEARCHUSERVIEW_H

#include <QListView>

/**
* @description: SearchUserView
* @author: cc
**/
class SearchUserView : public  QListView
{
public:
    explicit SearchUserView(QWidget* parent = nullptr);

protected:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;
};

#endif //STALK_V2_SEARCHUSERVIEW_H
