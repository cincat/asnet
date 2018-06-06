#include <thread_pool.h>

namespace asnet {
    ThreadPool::ThreadPool(int num) : index_(0), num_(num){
        for (int i = 0; i < num; i++) {
            Thread *t = new Thread();
            threads_.push_back(t);
        }
    }

    ThreadPool::~ThreadPool() {
        for (auto t : threads_) {
            delete t;
        }
    }
    
    void ThreadPool::start() {
        for (auto t : threads_) {
            t->run();
        }
    }
    Stream *ThreadPool::newStream() {
        if (threads_.size() == 0) return nullptr;
        return threads_[(index_++) % threads_.size()]->newStream();
    }
}// end of asnet