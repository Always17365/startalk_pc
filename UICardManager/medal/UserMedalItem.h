//
// Created by cc on 2019/10/21.
//
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#ifndef STALK_V2_USERMEDALITEM_H
#define STALK_V2_USERMEDALITEM_H

#include <QFrame>
#include "../../entity/im_medal_list.h"
#include "include/CommonStrcut.h"


/**
* @description: UserMedalItem
* @author: cc
* @create: 2019-10-21 19:56
**/
class UserMedalItem : public QFrame {
    Q_OBJECT
public:
    explicit UserMedalItem(QWidget* parent = nullptr);

public:
    void setUserMedal(const st::entity::ImMedalList& medalInfo,
                      int status,
                      bool isDetail);

    void setComingSoon();

protected:
    void paintEvent(QPaintEvent * e) override;

public:
    int _medalId = -1;

private:
    QString _imagePath;
    QString _text;
    bool    _isDetail;
};

#endif //STALK_V2_USERMEDALITEM_H
