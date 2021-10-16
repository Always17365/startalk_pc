//
// Created by cc on 18-12-24.
//

#ifndef STALK_V2_SESSIONBTN_H
#define STALK_V2_SESSIONBTN_H


#include <QToolButton>

/**
 * 会话按钮 显示未读数
 */
class SessionBtn : public QToolButton
{
public:
    SessionBtn(QWidget* parnet);
    ~SessionBtn();

public:
    void setUnreadCount(int count);

protected:
    void paintEvent(QPaintEvent* e) override;
    bool event(QEvent* e) override ;

private:
    int _unreadCount{};
    bool _isPressed;
};


#endif //STALK_V2_SESSIONBTN_H
