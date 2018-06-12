#include <log.h>
#include <sys/time.h>

#include <iostream>

using namespace asnet;

const int N = 10000;

int main() {

    struct timeval bval, eval;
    gettimeofday(&bval, nullptr);

    for (int i = 0; i < N; i++) {
        LOG_INFO << i << "hello, world\n";
    }

    gettimeofday(&eval, nullptr);
    std::cout << (N * 50) / (eval.tv_sec + eval.tv_usec/1e6 - bval.tv_sec -bval.tv_usec/1e6) << std::endl;
}