
#include <string.h>

#include <iostream>

#include <memory_pool.h>

int main() {
    asnet::MemoryPool pool;
    for (int i = 0; i < 100000; i++) {
        char *ptr = pool.allocate(16);
        strcpy(ptr, "hello, world!");
        std::cout << ptr << std::endl;
        pool.deallocate(ptr);
    }
    // char *ptr = pool.allocate(10000000000);
    return 0;
}