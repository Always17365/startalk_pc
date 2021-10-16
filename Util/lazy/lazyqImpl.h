#ifndef LAZYQIMPL_H
#define LAZYQIMPL_H

#include <functional>
#include <memory>
#include <atomic>
#include <string>
#include "Util/util_global.h"

using LazyImplCallback = std::function<void()>;

class STALK_UTIL_EXPORT lazyqImpl
{
public:
    lazyqImpl(int delayMs, const LazyImplCallback &func);

public:
    void addTask();

private:
    void helperCallback();

private:
    LazyImplCallback _callback;
    std::atomic_bool _deal {true};
    std::string      _uuid;
    int              _delay {100};
    long long    _t;
};

#endif // LAZYQIMPL_H
