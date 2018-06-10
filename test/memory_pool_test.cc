
#include <string.h>
#include <sys/time.h>

#include <iostream>

#include <memory_pool.h>

int main() {
    struct timeval ta, tb;
    gettimeofday(&ta, nullptr);
    asnet::MemoryPool pool;
    for (int i = 0; i < 100000; i++) {
        char *ptr = pool.allocate(16);
        strcpy(ptr, "hello, world!");
        // std::cout << ptr << std::endl;
        pool.deallocate(ptr);
    }
    ::gettimeofday(&tb, nullptr);
    std::cout << (tb.tv_sec + tb.tv_usec / 1e6) - (ta.tv_sec + ta.tv_usec/1e6) << std::endl;
    // char *ptr = pool.allocate(10000000000);
    return 0;
}