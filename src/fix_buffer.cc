
#include <unistd.h>
#include <string.h>

#include <fix_buffer.h>

namespace asnet {
FixBuffer::FixBuffer(): head_(0), tail_(0), left_(kBUFFER_LENGTH), length_(kBUFFER_LENGTH){
}

int FixBuffer::hasRoomFor(int n) {
    return left_ >= n;
}

void FixBuffer::append(const std::string &s) {
    int n = s.size();
    ::strncpy(buffer_ + tail_, s.data(), n);
    tail_ += n;
    left_ -= n;
}

bool FixBuffer::hasNothing() {
    return head_ == tail_;
}

void FixBuffer::reset() {
    head_ = 0;
    tail_ = 0;
    left_ = kBUFFER_LENGTH;
}
void FixBuffer::writeTo(int fd) {
    int n = ::write(fd, buffer_ + head_, tail_ - head_);
    if (n < 0) {
        return ;
    }
    head_ += n;
}
}// end of namespace asnet;