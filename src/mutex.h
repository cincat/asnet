#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>
#include <log.h>

#include <errno.h>

namespace asnet {
class Mutex {
public:
    Mutex() {
        int err = 0;
        err = pthread_mutex_init(&mutex_, nullptr);
        if (err < 0) {
            LOG_ERROR << strerror(errno) << "\n";
        }
    }
    void lock() {
        pthread_mutex_lock(&mutex_);
    }
    void unlock() {
        pthread_mutex_unlock(&mutex_);
    }
    ~Mutex() {
        pthread_mutex_destroy(&mutex_);
    }
private:
    pthread_mutex_t mutex_;
};

class MutexLock {
public:
    MutexLock(Mutex &mutex) :mutex_(mutex) {
        mutex_.lock();
    }
    ~MutexLock() {
        mutex_.unlock();
    }
private:
    Mutex &mutex_;
};
}// end of namespace asnet

#endif
