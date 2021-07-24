//
// Created by may on 2019-03-18.
//

#ifndef STLAZYQUEUE_H
#define STLAZYQUEUE_H

#include <mutex>
#include <queue>
#include <thread>
#include <iostream>
#include <functional>
#include <atomic>
#include <condition_variable>
#include "../QtUtil/Utils/Log.h"
#include "../include/threadhelper.h"

const unsigned long INTERVAL_MS = 10 ;

template<typename T>
class STLazyQueue
{
public:
    inline void init(unsigned long delayMs, std::function<int(STLazyQueue<T> *)> &func)
    {
        localFunc = [this, func, delayMs]()
        {
            _run.store(true);

            while (true)
            {
                if(!_run.load())
                    break;

                if(empty())
                {
                    std::unique_lock <std::mutex> lck(_waitMutex);
                    _wait.wait(lck);
                }

                unsigned long size = this->size();
                const std::chrono::milliseconds ms(delayMs);
                std::this_thread::sleep_for(ms);

                if(!_run.load())
                    break;

                unsigned long newSize = this->size();

                if (size != newSize)
                    continue;
                else
                {
                    if(!_run.load())
                        break;

                    if (!empty())
                    {
                        func(this);

                        // clear
                        if(!empty()) clear();
                    }
                }
            }
        };
    }

    explicit STLazyQueue(std::function<int(STLazyQueue<T> *)> &func)
    {
        init(INTERVAL_MS, func);
    }

    STLazyQueue(unsigned long delayMs, std::function<int(STLazyQueue<T> *)> &func)
    {
        init(delayMs, func);
    }

    ~STLazyQueue()
    {
        _run.store(false);
        _wait.notify_all();

        if(_thread)
            _thread->join();
    }

    void checkRunner()
    {
        if(_run.load() && m_queque.size() == 1)
        {
            if(nullptr == _thread)
                _thread = new std::thread(localFunc);

            _wait.notify_one();
        }
    }

    void push(const T &value)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        m_queque.push(value);
        checkRunner();
    }

    unsigned long size()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return m_queque.size();
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return m_queque.empty();
    }

    T tail()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return m_queque.back();
    }

    T front()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return m_queque.front();
    }

    void pop()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        m_queque.pop();
    }

    void clear()
    {
        std::lock_guard<std::mutex> lock(_mutex);

        while(!m_queque.empty()) m_queque.pop();
    }

private:
    unsigned long         _delayMs{0};
    std::function<void()> localFunc;
    std::queue<T>         m_queque;
    mutable std::mutex    _mutex;

    std::atomic<bool>     _run {true};
    std::mutex            _waitMutex;
    std::thread          *_thread{nullptr};
    std::condition_variable _wait;
};

#endif //STLAZYQUEUE_H
