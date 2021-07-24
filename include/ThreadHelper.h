#ifndef THREADHELPER_H
#define THREADHELPER_H

#include <thread>
#include <functional>

class ThreadHelper
{
public:
    inline ~ThreadHelper()
    {
        _thead->join();
    }

public:
    inline void run(const std::function<void()> &func)
    {
        _thead = new std::thread(func);
    }

private:
    std::thread *_thead{nullptr};
};

#endif // THREADHELPER_H
