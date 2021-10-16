//
// Created by cc on 18-12-17.
//

#ifndef STALK_V2_OAMAINWIDGET_H
#define STALK_V2_OAMAINWIDGET_H

#include <QFrame>

class OAManagerPanel;
class OAMainWidget : public QFrame
{
public:
    OAMainWidget(OAManagerPanel* panel);
    ~OAMainWidget();

private:
    void initUi();

private:
    OAManagerPanel *_pMainPanel;
};


#endif //STALK_V2_OAMAINWIDGET_H
