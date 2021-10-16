#include "HangManagerProcess.h"

#include <QDebug>
#include <QApplication>

static const char *processName = "HangMonitor.exe";

HangManagerProcess::HangManagerProcess(QObject *parent)
    : QObject(parent)
{
    connect(&_process, &QProcess::stateChanged, this,
            &HangManagerProcess::processStateChanged);
    connect(&_process, &QProcess::readyRead, this,
            &HangManagerProcess::onReadReady);
}

HangManagerProcess *HangManagerProcess::instance()
{
    static HangManagerProcess ins;
    return &ins;
}

void HangManagerProcess::processStateChanged(QProcess::ProcessState state)
{
    qInfo() << state;
}

void HangManagerProcess::onReadReady()
{
    qInfo() << _process.readAll();
}

void HangManagerProcess::start()
{
    QStringList args;
    args << QString::number(QApplication::applicationPid());

    _process.start(QString("%1/%2").arg(QApplication::applicationDirPath(),
                                        processName), args);
}
