#ifndef HANGMANAGERPROCESS_H
#define HANGMANAGERPROCESS_H

#include <QObject>
#include <QProcess>

class HangManagerProcess : public QObject
{
    Q_OBJECT
private:
    explicit HangManagerProcess(QObject *parent = nullptr);

public:
    HangManagerProcess *instance();
    void start();

protected:
    void processStateChanged(QProcess::ProcessState state);
    void onReadReady();

private:
    QProcess _process;

};

#endif // HANGMANAGERPROCESS_H
