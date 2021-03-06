#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <limits>

#include <stream.h>
#include <log.h>
#include <event_loop.h>


namespace asnet {

    bool streamComp(Stream *as, Stream *bs) {
        // pay attention getExpiredTimeAsMicroscends may same
        if (as->getExpireTime() == bs->getExpireTime()) {
            return as - bs;
        }
        return as->getExpireTime() - bs->getExpireTime();
    }

    // not thread safe
    void Stream::write() {
        out_buffer_.writeTo(fd_);
    }

    // not thread safe, must do in same thread
    void Stream::write(char *ptr, int len) {
        out_buffer_.append(ptr, len);
    }

    void Stream::write(Buffer *buffer) {
        buffer->writeTo(fd_);
    }

    void Stream::read() {
        in_buffer_.readFrom(fd_);
    }

    int Stream::read(char *buffer, int len) {
        return in_buffer_.subtract(buffer, len);
    }

    void Stream::addCallback(Event ev, Callback callback) {
        // fix me: log error information
        if (callbacks_[ev] != nullptr) {
            // return ;
            LOG_INFO << "replace old callback\n";
        }
        callbacks_[ev] = callback;
    }

    void Stream::listen(int port) {

        int fd = socket(AF_INET, SOCK_STREAM, 0);
        // fix me: add log information
        if (fd < 0) {
            return ;
        }

        struct sockaddr_in local;
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        local.sin_port = htons(port);

        int err = 1;

        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &err, sizeof(int));
        err = ::bind(fd, (struct sockaddr *)&local, sizeof(local));
        if (err < 0) {
            // fix me:
            LOG_ERROR << "error occurs during bind: " << strerror(errno) << "\n";
            return ;
        }

        // int flags = fcntl(fd, F_GETFL);
        // err = fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        // if (err < 0) {
        //     LOG_ERROR << "set unblocking socket fd failed\n";
        // }
        err = ::listen(fd, 500);
        if (err < 0) {
            LOG_ERROR << strerror(errno) << "\n";
        }

        setState(State::LISTENING);
        setFd(fd);
    }

    void Stream::connect(char *addr, int port) {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            return ;
        }
        setFd(fd);

        struct sockaddr_in remote;
        remote.sin_family = AF_INET;
        remote.sin_port = htons(port);
        int err = 0;
        err = inet_aton(addr, &remote.sin_addr);
        if (err < 0) {
            LOG_ERROR << "change ip address from input failed\n";
            return ;
        }

        int flags = fcntl(fd, F_GETFL);
        err = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
        if (err < 0) {
            LOG_ERROR << "set unblocking socker fd failed\n";
        }
        err = ::connect(fd, (struct sockaddr*)&remote, sizeof(remote));
        
        if (err < 0 && errno != EINPROGRESS) {
            LOG_ERROR << "connect to remote address failed:" << strerror(errno);
        }

        // LOG_INFO << "successfully listened on port " << port << "\n";
        setState(State::CONNECTING);
    }

    uint64_t Stream::getCurrentTimeAsMilliscends() {
        struct timeval current_time;
        int err = gettimeofday(&current_time, nullptr);
        if (err < 0) {
            LOG_ERROR << "get current time failed: " << strerror(errno) << "\n";
            return -1;
        }
        double time_as_mirosenconds = current_time.tv_sec * 1000 + current_time.tv_usec / 1000;
        return time_as_mirosenconds;
    }


    
    // note: timeout and tiktok unit is second.
    void Stream::runAfter(int timeout, Callback callback) {
        timeout_ = timeout * 1000;
        addCallback(Event::TIMEOUT, callback);
        last_timeout_ = getCurrentTimeAsMilliscends();
        loop_->adjustStream(this);
    }

    void Stream::runEvery(int ticktock, Callback callback) {

        ticktock_ = ticktock * 1000;
        addCallback(Event::TICKTOCK, callback);
        last_ticktock_ = getCurrentTimeAsMilliscends();
        loop_->adjustStream(this);
    }

    uint64_t Stream::getTimeoutExpireTime() {
        uint64_t expire = last_timeout_ + timeout_;
        if (expire == 0) {
            return std::numeric_limits<long long>::max();
        }
        return expire;
    }

    uint64_t Stream::getTicktockExpireTime() {
        uint64_t expire = last_ticktock_ + ticktock_;
        if (expire == 0) {
            return std::numeric_limits<long long>::max();
        }
        return expire;
    }

    uint64_t Stream::getExpireTime() {
        return std::min(getTimeoutExpireTime(), getTicktockExpireTime());
    }
    void Stream::close() {
        LOG_INFO << "waiting to close\n";
        setState(State::CLOSING);
        if (hasCallbackFor(Event::TIMEOUT)) {
            timeout_ = 0;
            last_timeout_ = 0;
            callbacks_[Event::TIMEOUT] = nullptr;
        }
        if (hasCallbackFor(Event::TICKTOCK)) {
            ticktock_ = 0;
            last_ticktock_ = 0;
            callbacks_[Event::TICKTOCK] = nullptr;
        }
        loop_->createEvent();
    }
}// end of namespace asnet