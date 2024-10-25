//
// Created by cc on 2019/10/17.
//

#ifndef STALK_V2_MEDALWGT_H
#define STALK_V2_MEDALWGT_H

#include <QFrame>
#include <set>
#include <map>
#include "customui_global.h"
#include "entity/im_medal_list.h"
#include "include/CommonStrcut.h"

/**
* @description: MedalWgt
* @author: cc
* @create: 2019-10-17 15:23
**/
class CUSTOMUISHARED_EXPORT MedalWgt : public QFrame
{
    Q_OBJECT
public:
    explicit MedalWgt(int icon_width, QWidget *parent = nullptr);
    ~MedalWgt() override;

public:
    void addMedal(int medal_id);
    void addMedals(const std::set<st::StUserMedal> &user_medal, bool = false);
    void removeMedal(int medal_id);

protected:
    void setHeight();

protected:
    void paintEvent(QPaintEvent *e) override ;

private:
    std::map<int, st::entity::ImMedalList> _medals;
    int _icon_width = 0;
};


#endif //STALK_V2_MEDALWGT_H
