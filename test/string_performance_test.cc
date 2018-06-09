
#include <sys/time.h>

#include <iostream>
// #include <vector>

int main() {
    struct timeval ta, tb;
    gettimeofday(&ta, nullptr);
    std::string buffer;
    for (int i = 0; i < 1000000; i++) {
        buffer.append("hello, world!");
    }
    ::gettimeofday(&tb, nullptr);
    std::cout << (tb.tv_sec + tb.tv_usec/1e6) - (ta.tv_sec + ta.tv_usec/1e6) << std::endl;
    // char *ptr = pool.allocate(10000000000);
    return 0;
}