#include <log.h>
// #include <log_stream.h>

#include <sys/time.h>
#include <pthread.h>

#include <iostream>

using namespace asnet;

const int N = 10000;
void *threadFunc(void *arg) {
    // const int N = 10000;
    int64_t pi = (int64_t)arg;
    for (int i = 0; i < N; i++) {
        LOG_INFO << "thread " << pi << " " << i << "hello, world\n";
    }
}

int main() {
    // for(int i = 0; i < 1000; i++) {
    //     LOG_INFO << "hello, world\n";
    // }
    // const int N = 100000;
    struct timeval bval, eval;
    gettimeofday(&bval, nullptr);
    // for (int i = 0; i < N; i++) {
    //     LOG_INFO << i << "hello, world\n";
    // }

    pthread_t threads[3];
    for (int i = 0; i < 3; i++) {
        pthread_create(threads + i, nullptr, threadFunc, (void*)i);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], nullptr);
    }
    gettimeofday(&eval, nullptr);
    std::cout << (N * 55) / (eval.tv_sec + eval.tv_usec/1e6 - bval.tv_sec -bval.tv_usec/1e6) << std::endl;
}