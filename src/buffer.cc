
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include<buffer.h>
#include<memory_pool.h>
#include<log.h>

namespace asnet {
    Buffer::Buffer(MemoryPool *pool) :
        buffer_(nullptr),
        head_(0),
        tail_(0),
        length_(0),
        pool_(pool) {}

    void Buffer::append(char *ptr, int n) {
        ensureCapacity(n);
        memcpy(buffer_ + tail_, ptr, n);
        tail_ += n;
        return ;
    }

    bool Buffer::hasContent() {
        return tail_ > head_;
    }

    void Buffer::writeTo(int fd) {
        int n = ::write(fd, buffer_ + head_, tail_ - head_);
        if (n < 0) {
            LOG_ERROR << strerror(errno) << "\n";
        }
        else {
            head_ += n;
            if (head_ == tail_) {
                head_ = tail_ = 0;
            }
        }
    }

    void Buffer::readFrom(int fd) {
        
    }

    void Buffer::ensureCapacity(int n) {
        if (buffer_ == nullptr) {
            buffer_ = pool_->allocate(2 * n);
            head_ = 0;
            tail_ = 0;
            length_ = 2 * n;
            return ;
        }
        else if (tail_ + n > length_) {
            char *ptr = pool_->allocate(2 * (tail_ - head_ + n));
            memcpy(ptr, buffer_ + head_, tail_ - head_);
            pool_->deallocate(buffer_);
            buffer_ = ptr;
            tail_ -= head_;
            head_ = 0;
            length_ = 2 * (tail_ + n);
        }
    }

}// end of namespace asnet