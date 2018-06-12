#ifndef CONDITION_H
#define CONDITION_H

#include <mutex.h>
#include <pthread.h>
#include <sys/time.h>

namespace asnet {

class Condition {
public:
    Condition(Mutex &mutex) : mutex_(mutex){
        pthread_cond_init(&cond_, nullptr);
    }
    ~Condition() {
        pthread_cond_destroy(&cond_);
    }
    void wait() {
        pthread_cond_wait(&cond_, mutex_.getInternalMutex());
    }

    // wait at most t seconds
    void wait(int t) {
        struct timeval tval;
        gettimeofday(&tval, nullptr);
        struct timespec tspec;
        tspec.tv_sec = tval.tv_sec;
        tspec.tv_nsec = tval.tv_usec * 1000;
        tspec.tv_sec += t;
        pthread_cond_timedwait(&cond_, mutex_.getInternalMutex(), &tspec);
    }
    
    void notify() {
        pthread_cond_signal(&cond_);
    }
    void notifyAll() {
        pthread_cond_broadcast(&cond_);
    }
private:
    Mutex &mutex_;
    pthread_cond_t cond_;
};
}// end of namespace asnet

#endif