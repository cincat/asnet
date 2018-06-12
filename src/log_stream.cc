
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <log_stream.h>

#include <iostream>

namespace asnet {

void *threadFunc(void *p) {
    LogStream *log = static_cast<LogStream *>(p);
    while (true) {log->flush();}
    return nullptr;
}

void LogStream::doFlush() {

    if (primeBuffer_->hasNothing() == false) {
        work_list_.push_back(primeBuffer_);
        if (free_list_.empty() == false) {
            primeBuffer_ = free_list_.front();
            free_list_.pop_front();
        }
        else {
            primeBuffer_ = new FixBuffer();
        }

        while (work_list_.size()) {
            auto buffer_ = work_list_.front();
            work_list_.pop_front();
            while (buffer_->hasNothing() == false) {
                buffer_->writeTo(fd_);
            }
            buffer_->reset();
            free_list_.push_back(buffer_);
        }
    }
}

void LogStream::flush() {
    MutexLock lock(mutex_);
    if (work_list_.empty() == true) {
        condition_.wait(5);
    }
    doFlush();
}

LogStream::LogStream() :
    fd_(STDERR_FILENO), 
    mutex_(),
    condition_(mutex_),
    primeBuffer_(new FixBuffer()){
        // log_thread_ = pthread_create()
        int err = ::pthread_create(&log_thread_, nullptr, threadFunc, this);
}


LogStream::LogStream(const std::string &s) : LogStream(){
    fd_ = open(s.data(), O_APPEND | O_CREAT);
    if (fd_ < 0) {
        // error checking
    }
}

void LogStream::append(const std::string &s) {
    MutexLock lock(mutex_);
    if (primeBuffer_->hasRoomFor(s.size())) {
        primeBuffer_->append(s);
    }
    else {
        work_list_.push_back(primeBuffer_);
        condition_.notify();
        if (free_list_.empty() == false) {
            primeBuffer_ = free_list_.front();
            free_list_.pop_front();
        }
        else {
            primeBuffer_ = new FixBuffer();
        }
        
        if (primeBuffer_->hasRoomFor(s.size())) {
            primeBuffer_->append(s);
        }
    }
}

}// end of namespace asnet