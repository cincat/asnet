
#include <errno.h>
#include <string.h>

#include <memory_pool.h>
#include <log.h>

namespace asnet {

    MemoryPool::MemoryPool(int block_size) : unit_(block_size){
        // work_list_.push_back(std::move());
    }

    MemoryPool::MemoryPool() : MemoryPool(65536) {}
    void MemoryPool::allocateNewBlock(int n) {
        if (n > unit_/4) {
            work_list_.push_back(MemoryBlock());
            MemoryBlock &block = work_list_.back();
            block.head = new char[n];
            block.ptr = block.head;
            if (block.head == nullptr) {
                LOG_ERROR << "malloc failed: " << strerror(errno) << "\n";
            }
            block.unused = n;
            block.length = n;
            block.refund = block.length;
            LOG_INFO << "MemoryPool has allocated a new " << n << " bytes block\n";
            return ;
        }
        else {
            if (free_list_.size() == 0) {
                work_list_.push_back(MemoryBlock());
                MemoryBlock &block = work_list_.back();
                block.head = new char[unit_];
                block.ptr = block.head;
                if (block.head == nullptr) {
                    LOG_ERROR << "malloc failed: " << strerror(errno) << "\n";
                }
                block.unused = unit_;
                block.length = unit_;
                block.refund = unit_;
                LOG_INFO << "MemoryPool has allocated a new 64k block\n";
            }
            else {
                auto block = free_list_.back();
                block.ptr = block.head;
                block.unused = unit_;
                block.refund = unit_;
                work_list_.push_back(block);
                free_list_.pop_back();
                LOG_INFO << "MemoryPool has transfer a new block from free section\n";
            }
        }
        
    }

    char *MemoryPool::allocate(int n) {
        
        if (work_list_.empty() || work_list_.back().unused < n){
            if (work_list_.empty() == false) {
                auto &t = work_list_.back();
                t.refund = t.ptr - t.head;
            }
            allocateNewBlock(n);
            MemoryBlock &block = work_list_.back();
            block.ptr += n;
            block.unused -= n;
            record_[block.head] = std::make_pair(&block, n);
            LOG_INFO << "MemoryPool has allocated " << n << " bytes on a new block\n";
            return block.head;
        }
        else {
            MemoryBlock &block = work_list_.back();
            char *pre_ptr = block.ptr;
            block.ptr += n;
            block.unused -= n;
            record_[pre_ptr] = std::make_pair(&block, n);
            LOG_INFO << "MemoryPool has allocated " << n << " bytes on an existed block\n";
            return pre_ptr;
        }
    }

    void MemoryPool::deallocate(char *ptr) {
        if (record_.count(ptr) == 0) {
            LOG_ERROR << "invalid address should not deallocated\n";
            return ;
        }

        MemoryBlock *block = record_[ptr].first;
        block->refund -= record_[ptr].second;
        LOG_INFO << "MemoryPool has " << record_[ptr].second << " bytes returned\n";
        if (block->refund == 0) {
            for (auto it = work_list_.begin(); it != work_list_.end(); it++) {
                if (block == &(*it)) {
                    if (block->length == unit_) {
                        LOG_INFO << "transfer a block to free section\n";
                        free_list_.push_back(*it);
                        work_list_.erase(it);

                    }
                    else {
                        LOG_INFO << "delete a irregular block\n";
                        work_list_.erase(it);
                        delete[] block->head;
                    }
                    break;
                }
            }
        }
        record_.erase(ptr);
        return ;
    }

    MemoryPool::~MemoryPool() {
        for (auto block : work_list_) {
            LOG_INFO << "returning heap memory\n";
            delete[] block.head;
        }
        for (auto block : free_list_) {
            LOG_INFO << "returning heap memory\n";
            delete[] block.head;
        }
    }

}// end of namespace asnet