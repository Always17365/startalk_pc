//
// Created by cc on 18-12-10.
//

#ifndef STALK_V2_SYSTEMTRAY_H
#define STALK_V2_SYSTEMTRAY_H


#include <QObject>
#include <QSystemTrayIcon>
#include <QTimer>
#include "include/CommonStrcut.h"
#include "SystemTrayPopWnd.h"

class MainWindow;
class SystemTray : public QObject
{
    Q_OBJECT
public:
    explicit SystemTray(MainWindow* mainWnd);
    ~SystemTray() override;

public slots:
    void onShowNotify(const st::StNotificationParam&);
    void onAppDeactivated();

Q_SIGNALS:
    void sgShowUnreadMessage(int, const st::entity::UID&, const QString&, qint64, int);

public:
    void onWndActived();
    void onSendLog();

protected:
    void activeTray(QSystemTrayIcon::ActivationReason reason);
    void onMessageClicked();

private:
    void onTimer();
    void stopTimer();
    void onHoverTimer();

private:
    MainWindow*      _pMainWindow{};
    QSystemTrayIcon* _pSysTrayIcon{};
    QTimer*          _timer{};

    SystemTrayPopWnd *_popWnd{};
    QMenu            *pSysTrayMenu{};

private:
    unsigned short  _timerCount;
};


#endif //STALK_V2_SYSTEMTRAY_H
