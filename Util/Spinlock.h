//
// Created by may on 2019-02-12.
//

#ifndef STALK_V2_SPINLOCK_H
#define STALK_V2_SPINLOCK_H

#include <atomic>
#include <thread>
#include <mutex>
#include <iostream>

namespace st {
    namespace util {
        class spin_mutex {
            std::atomic_flag flag = ATOMIC_FLAG_INIT;
        public:
            spin_mutex() = default;

            spin_mutex(const spin_mutex &) = delete;

            spin_mutex &operator=(const spin_mutex &) = delete;

            void lock() {
                while (flag.test_and_set(std::memory_order_acquire));
            }

            void unlock() {
                flag.clear(std::memory_order_release);
            }
        };
    }
}

#endif //STALK_V2_SPINLOCK_H
