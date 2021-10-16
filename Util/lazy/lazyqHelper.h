#ifndef DELAYTASKHELPER_H
#define DELAYTASKHELPER_H

#include <QThread>
#include <QMap>
#include <QMutexLocker>
#include <functional>
#include <atomic>

using TaskMap = QMap<qint64, QHash<QString, std::function<void()>>>;

class LazyQHelper : public QThread
{
public:
    static LazyQHelper *instance();
    ~LazyQHelper();

public:
    void addTask(qint64 t,
                 const std::string &uuid,
                 const std::function<void()> &callback);

    void removeTask(qint64 t, const std::string &uuid);

protected:
    void run() override;

private:
    LazyQHelper();

private:
    TaskMap          _tasks;
    std::atomic_bool _run {false};

    QMutex _mutex;
};

#endif // DELAYTASKHELPER_H
