//
// Created by eduardo on 12/03/23.
//

#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace opcua {

/**
 * Thread safe queue taken from https://codereview.stackexchange.com/questions/267847/thread-safe-message-queue
 * @tparam T
 */
template <typename T>
class SafeQueue {
    std::mutex mutex;
    std::condition_variable cond_var;
    std::queue<T> queue;

public:
    void push(T&& item) {
        {
            std::lock_guard lock(mutex);
            queue.push(item);
        }

        cond_var.notify_one();
    }

    T& front() {
        std::unique_lock lock(mutex);
        cond_var.wait(lock, [&]{ return !queue.empty(); });
        return queue.front();
    }

    void pop() {
        std::lock_guard lock(mutex);
        T item;
        queue.pop(item);
    }
};

}  // namespace opcua
