//
// Created by cc on 2019-03-22.
//
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#ifndef STALK_V2_ADDGROUPEDIT_H
#define STALK_V2_ADDGROUPEDIT_H

#include <QTextEdit>

/**
* @description: AddGroupEdit
* @author: cc
* @create: 2019-03-22 17:31
**/
class AddGroupEdit : public QTextEdit {
	Q_OBJECT
public:
    explicit AddGroupEdit(QWidget* parent = nullptr);
    ~AddGroupEdit() override;

public:
    void keyPressEvent(QKeyEvent* e) override;
};


#endif //STALK_V2_ADDGROUPEDIT_H
