#include <block_queue.h>
#include <log.h>

#include <iostream>

int main() {
    asnet::BlockQueue<int> queue;
    for (int i = 0; i < 10; i++) {
        queue.push_back(i);
        // std::cout << queue.size() << std::endl;
    }

    // std::cout << queue.size() << std::endl;
    for (int i = 0; queue.size() != 0; i++) {
        std::cout << queue.front() << std::endl;
        queue.pop_front();
    }
    return 0;
}