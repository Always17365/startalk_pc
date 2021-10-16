#include "lazyqhelper.h"

#include <QDateTime>
#include <QMapIterator>

#include "Util/Log.h"
#include <QDebug>

LazyQHelper::LazyQHelper()
{
    this->start();
}

LazyQHelper::~LazyQHelper()
{
    _run.store(false);
    wait(1000);
}

LazyQHelper *LazyQHelper::instance()
{
    static LazyQHelper helper;
    return &helper;
}

void LazyQHelper::run()
{
    _run.store(true);

    while (_run.load()) {

        bool empty = false;
        {
            QMutexLocker locker(&_mutex);
            empty = _tasks.empty();
        }

        if (empty) {
            msleep(50);
            continue;
        }

        auto now = QDateTime::currentMSecsSinceEpoch();

        // 迭代器不会失效
        QMutexLocker locker(&_mutex);

        QMapIterator<qint64, QHash<QString, std::function<void()>>> it(_tasks);

        while (it.hasNext()) {
            it.next();

            if (now < it.key()) {
                break;
            }

            for (auto &fc : it.value().values()) {
                fc();
            }

            _tasks.remove(it.key());
        }
    }
}

void LazyQHelper::addTask(qint64 t,
                          const std::string &uuid,
                          const std::function<void ()> &callback)
{
    QMutexLocker locker(&_mutex);
    _tasks[t][uuid.data()] = callback;
}

void LazyQHelper::removeTask(qint64 t, const std::string &uuid)
{
    QMutexLocker locker(&_mutex);
    _tasks[t].remove(uuid.data());

    if (_tasks[t].isEmpty()) {
        _tasks.remove(t);
    }
}
