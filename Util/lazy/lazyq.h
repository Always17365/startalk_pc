#ifndef LAZYQ_H
#define LAZYQ_H

#include <queue>
#include <mutex>

#include "lazyqImpl.h"
#include "Util/util_global.h"

template<typename T>
class STALK_UTIL_EXPORT lazyq
{
public:
    using lazyqq = std::shared_ptr<std::queue<T>>;
    using LazyCallback = std::function<void(lazyqq &q)>;
    /**
     * @param delayMs 延时
     */
    lazyq(int delayMs, const LazyCallback &func)

    {
        _q = std::make_shared<std::queue<T>>();
        _callback = func;
        _impl = new lazyqImpl(delayMs, std::bind(&lazyq<T>::doCallBack, this));
    }

public:
    void push(const T &t)
    {
        _impl->addTask();

        std::lock_guard<std::mutex> lock(_mutex);
        _q->push(t);
    }

private:
    void doCallBack()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _callback(_q);

        while (!_q->empty()) {
            _q->pop();
        }
    }

private:
    lazyqq        _q;
    lazyqImpl    *_impl{nullptr};
    LazyCallback _callback;

    std::mutex   _mutex;
};

#endif // LAZYQ_H
