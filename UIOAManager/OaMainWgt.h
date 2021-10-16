//
// Created by cc on 18-12-17.
//
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#ifndef STALK_V2_OAMAINWIDGETITEM_H
#define STALK_V2_OAMAINWIDGETITEM_H

#include <QFrame>
#include <QTableWidget>
#include "include/CommonDefine.h"
#include "include/CommonStrcut.h"

typedef st::StOAUIData::StMember StMember;

/**
 *
 */
class CellWgtItem : public QFrame
{
	Q_OBJECT
public:
    CellWgtItem(QString name, QString icon, QWidget *parent = nullptr);
    ~CellWgtItem() override;

public:
    void mousePressEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override ;

Q_SIGNALS:
    void itemClicked();

private:
    QString _name;
    QString _icon;
};

class OAManagerPanel;
class OaMainWgt : public QFrame
{
	Q_OBJECT
public:
    OaMainWgt(const int& id, QString name, const std::vector<StMember>& member, QWidget* parent = nullptr);
    ~OaMainWgt() override;

private:
    void initUi();

Q_SIGNALS:
    void sgShowThrowingScreenWnd();

private:
    int _id;
    QString _name;
    const std::vector<StMember>& _members;

private:
    QTableWidget* _pTabWgt = nullptr;
};


#endif //STALK_V2_OAMAINWIDGETITEM_H
