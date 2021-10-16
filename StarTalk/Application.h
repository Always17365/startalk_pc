#pragma once

#include <QApplication>
#include <QMutexLocker>
#include "MainWindow.h"

class GlobalManager;
class LogicManager;
class DataCenter;

class Application : public QApplication
{
    Q_OBJECT

public:
    Application(int argc, char *argv[]);
    ~Application() override;

protected:
    void initLogSys();
    bool notify(QObject *receiver, QEvent *e) Q_DECL_OVERRIDE;
    bool event(QEvent *event) override;
    static void initTTF();

private:
    void onApplicationStateChange(Qt::ApplicationState state);

private:
    GlobalManager *_pUiManager  {nullptr};
    LogicManager *_pLogicManager{nullptr};

public:
    static QString _strlogPath;
    static QString _strqLogPath;

public:
    MainWindow *_pMainWnd;

};
