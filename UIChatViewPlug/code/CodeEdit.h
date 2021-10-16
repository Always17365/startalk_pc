//
// Created by cc on 2019-02-28.
//
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#ifndef STALK_V2_CODEEDIT_H
#define STALK_V2_CODEEDIT_H

#include <QTextEdit>

/**
* @description: CodeEdit
* @author: cc
* @create: 2019-02-28 13:44
**/
class CodeEdit : public QTextEdit{
	Q_OBJECT
public:
    explicit CodeEdit(QWidget* parent = nullptr);
    ~CodeEdit() override;

protected:
    void keyPressEvent(QKeyEvent* e) override;

};


#endif //STALK_V2_CODEEDIT_H
