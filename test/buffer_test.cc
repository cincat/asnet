
#include <unistd.h>

#include <buffer.h>
#include <memory_pool.h>

int main() {
    asnet::MemoryPool pool;
    asnet::Buffer buffer(&pool);
    for(int i = 0; i < 10; i++) {
        buffer.append("hello, world!\n", 14);
    }
    buffer.writeTo(STDOUT_FILENO);
    return 0;
}