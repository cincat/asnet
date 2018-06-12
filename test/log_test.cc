#include <log.h>
// #include <log_stream.h>

#include <sys/time.h>

#include <iostream>

using namespace asnet;

int main() {
    // for(int i = 0; i < 1000; i++) {
    //     LOG_INFO << "hello, world\n";
    // }
    const int N = 100000;
    struct timeval bval, eval;
    gettimeofday(&bval, nullptr);
    for (int i = 0; i < N; i++) {
        LOG_INFO << i << "hello, world\n";
    }
    gettimeofday(&eval, nullptr);
    std::cout << (N * 55) / (eval.tv_sec + eval.tv_usec/1e6 - bval.tv_sec -bval.tv_usec/1e6) << std::endl;
    
}