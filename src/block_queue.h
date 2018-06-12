
#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <deque>
#include <mutex.h>

namespace asnet {

// iterate is thread unsafe
template <typename T>
class BlockQueue {
public:
    BlockQueue() = default;

    void push_back(T &item) {
        MutexLock lock(mutex_);
        queue_.push_back(item);
    }

    void pop_back() {
        MutexLock lock(mutex_);
        queue_.pop_back();
    }

    void push_front(T &item) {
        MutexLock lock(mutex_);
        queue_.push_front(item);
    }

    void pop_front() {
        MutexLock lock(mutex_);
        queue_.pop_front();
    }

    bool empty() {
        MutexLock lock(mutex_);
        return queue_.empty();
    }

    size_t size() {
        MutexLock lock(mutex_);
        return queue_.size();
    }

    void clear() {
        MutexLock lock(mutex_);
        queue_.clear();
    }

    T &front() {
        MutexLock lock(mutex_);
        return queue_.front();
    }

    T &back() {
        MutexLock lock(mutex_);
        return queue_.back();
    }

    Mutex &getMutex() {
        return mutex_;
    }
    // typename std::deque<T>::iterator begin() {
    //     return queue_.begin();
    // }

    // typename std::deque<T>::iterator end() {
    //     return queue_.end();
    // }

    // Mutex &getLock() {
    //     return mutex_;
    // }
private:
    std::deque<T> queue_;
    Mutex mutex_;
};

}// end of asnet

#endif