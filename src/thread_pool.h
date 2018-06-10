#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <vector>

#include <thread.h>

namespace asnet {
class ThreadPool {
public:
    ThreadPool(int num);
    ~ThreadPool();
    Stream* newStream(int);
    void start();
    int size() {return num_;}
private:
    std::vector<Thread *> threads_;
    int index_;
    int num_;
};
}// end of namespace asnet

#endif