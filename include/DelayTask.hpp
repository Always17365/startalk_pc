//
// Created by cc on 2019/09/04.
//

#ifndef QTALK_V2_NETWORKCHECKTASK_H
#define QTALK_V2_NETWORKCHECKTASK_H

#include <functional>
#include <thread>
#include <mutex>
#include <memory>
#include <chrono>
#include <iostream>

/**
 * 延时任务
 * 1 _delay 延时时间
 * 2 _call_fun 回调函数 bool 返回值 true:继续延时_delay false:取消任务
 * 3
 */

using namespace std::chrono;
using std::chrono::steady_clock;

class DelayTask
{

public:
    explicit DelayTask(int delay, std::string name, std::function<bool()> fun)
        : _name(std::move(name)), _call_fun(std::move(fun)), _delay(delay)
    {}

    ~DelayTask()
    {
        _run.store(false);
        const milliseconds ms(200);
        std::this_thread::sleep_for(ms);

        if(_thread)
            _thread->join();
    }

    void start()
    {
        update();

        if(_run.load())
            return;

        run();
    }

    void update()
    {
        if(!_run.load())
            return;

        std::lock_guard<std::mutex> lock(_mutex);
        _execute_time = now();
    }

    void setDelay(int delay)
    {
        if(!_run.load())
            return;

        std::lock_guard<std::mutex> lock(_mutex);
        _delay = delay;
        _execute_time = now();
    }

    bool isRuning()
    {
        std::lock_guard<std::mutex> lock(_mutex);
        return _run.load();
    }

protected:
    void run()
    {
        if(_thread)
        {
            _run.store(false);
            _thread->join();
            delete _thread;
        }

        _run.store(true);
        _thread = new std::thread([this]()
        {
            while (_run.load())
            {
                thread_local bool call  = false;
                {
                    std::lock_guard<std::mutex> lock(_mutex);
                    call = (now() - _execute_time) > _delay;
                }

                if(_run.load() && call)
                {
                    if(_run.load() && _call_fun())
                        update();
                    else
                    {
                        _run.store(false);
                        break;
                    }
                }

                if(_run.load())
                {
                    const milliseconds ms(1000);
                    std::this_thread::sleep_for(ms);
                }
            }
        });
    }

    long long now ()
    {
        auto now = steady_clock::now();
        return duration_cast<milliseconds>(now.time_since_epoch()).count();
    }


private:

    std::mutex  _mutex;
    std::string _name;

    std::function<bool()>   _call_fun;
    std::thread            *_thread {nullptr};
    std::atomic<bool>       _run {false};
    int                     _delay {0};
    long long               _execute_time{0};
};

#endif //QTALK_V2_NETWORKCHECKTASK_H
