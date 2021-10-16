//
// Created by cc on 2020/2/12.
//

#ifndef STALK_V2_NTMESSAGEBOX_H
#define STALK_V2_NTMESSAGEBOX_H

#include "UShadowWnd.h"
#include "customui_global.h"

class CUSTOMUISHARED_EXPORT NTMessageBox : public UShadowDialog {
    Q_OBJECT
public:
    NTMessageBox(QWidget* parent, const QString& message, int buttons);
    ~NTMessageBox() override ;

public:
    enum
    {
        EM_BUTTON_INVALID = 0,
        EM_BUTTON_YES = 1,
        EM_BUTTON_NO = 2,
    };

public:
    static int showMessage(QWidget* parent, const QString& message, int buttons = EM_BUTTON_YES);

protected:
    void keyPressEvent(QKeyEvent* e) override;

private:
    int _retButton;
    QEventLoop* _evtLoop;

private:
    QLabel *mainMessageLabel;
};

#endif //STALK_V2_NTMESSAGEBOX_H
