#pragma once

#include <QApplication>
#include <QMutexLocker>
#include "MainWindow.h"

class GlobalManager;

class LogicManager;

class Platform;

class QTalkApp : public QApplication {
Q_OBJECT

public:
    QTalkApp(int argc, char *argv[]);

    ~QTalkApp() override;

protected:
    static void initLogSys();
    bool notify(QObject *receiver, QEvent *e) Q_DECL_OVERRIDE;
    bool event(QEvent *event) override;
    static void initTTF();

private:
    void onApplicationStateChange(Qt::ApplicationState state);

private:
    GlobalManager *_pUiManager{};
    LogicManager *_pLogicManager{};

public:
    MainWindow *_pMainWnd;

};
