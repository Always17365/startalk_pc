//
// Created by cc on 2018-12-25.
//

#ifndef STALK_V2_NAVMANAGER_H
#define STALK_V2_NAVMANAGER_H

#include "CustomUi/UShadowWnd.h"
#include <QPushButton>
#include <QMap>
#include "Util/ini/ConfigLoader.h"
#include "NavView.h"

class LoginPanel;
class NavManager : public UShadowDialog
{
    Q_OBJECT
public:
    explicit NavManager(LoginPanel *loginPanel);

public:
    QString getDefaultNavUrl();
    QString getNavName();
    QString getDefaultDomain();

protected:
    void initConfig();
    void initUi();
    void saveConfig();
    void onSaveConf();
    void onAddNav(const QString &name, const QString &navAddr, const bool &isDebug);
    void onNavChanged();

private:
    LoginPanel          *_pLoginPanel;
    NavView             *_pNavView{};

private://data
    QMap<QString, StNav> _mapNav;
    QString              _defaultKey;

private:
    QPushButton *_pCloseBtn{}; // 关闭按钮

};


#endif //STALK_V2_NAVMANAGER_H
