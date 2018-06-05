#include <errno.h>

#include <thread.h>
#include <log.h>

namespace asnet {
    void *startThread(void *args) {
        Thread *t = static_cast<Thread*>(args);
        t->threadFunc();
        return nullptr;
    }
    Thread::Thread() :loop_(), mutex_(), cond_(mutex_){
        int err = 0;
        err = pthread_create(&thread_, nullptr, startThread, this);
        if (err < 0) {
            LOG_ERROR << strerror(errno) << "\n";
        }
    }

    void Thread::threadFunc() {
        cond_.wait();
        loop_.run();
    }

    void Thread::run() {
        cond_.notify();
    }

    EventLoop *Thread::getEventLoop() {
        return &loop_;
    }
}