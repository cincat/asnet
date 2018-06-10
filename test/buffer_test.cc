
#include <unistd.h>

#include <buffer.h>
#include <memory_pool.h>

int main() {
    struct timeval ta, tb;
    gettimeofday(&ta, nullptr);
    asnet::MemoryPool pool;
    asnet::Buffer buffer(&pool);
    for(int i = 0; i < 1000000; i++) {
        buffer.append("hello, world!\n", 14);
    }
    // buffer.writeTo(STDOUT_FILENO);
    ::gettimeofday(&tb, nullptr);
    std::cout << (tb.tv_sec + tb.tv_usec/1e6) - (ta.tv_sec + ta.tv_usec/1e6) << std::endl;
    return 0;
}