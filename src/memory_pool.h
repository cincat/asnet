#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <list>
#include <unordered_map>

#include <errno.h>

#include <log.h>

namespace asnet {
struct MemoryBlock {
    char *head;
    char *ptr;
    int length;
    int unused;
    int refund;
    MemoryBlock(): head(nullptr), ptr(nullptr), length(0), unused(0), refund(0){}
};

class MemoryPool {

public:
    MemoryPool(int);
    MemoryPool();
    ~MemoryPool();

    MemoryPool(const MemoryPool&) = delete;
    MemoryPool &operator=(const MemoryPool&) = delete;

    char *allocate(int);
    void deallocate(char *);

private:

    void allocateNewBlock(int);

    std::list<MemoryBlock> free_list_;
    std::list<MemoryBlock> work_list_;
    std::unordered_map<char *, std::pair<MemoryBlock*, int>> record_;
    int unit_;
};

}// end of namespace asnet

#endif