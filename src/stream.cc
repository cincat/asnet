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


namespace asnet {

    bool streamComp(Stream *as, Stream *bs) {
        // pay attention getExpiredTimeAsMicroscends may same
        if (as->getExpiredTimeAsMicroscends() == bs->getExpiredTimeAsMicroscends()) {
            return as - bs;
        }
        return as->getExpiredTimeAsMicroscends() - bs->getExpiredTimeAsMicroscends();
    }

    // not thread safe
    void Stream::write() {
        // int n = 0;
        // MutexLock lock(mutex_);
        // n = ::write(fd_, write_buffer_, write_index_);
        // if (n < 0) {
        //     return n;
        // }
        // LOG_INFO << "successfully write " << n << " bytes on fd " << fd_ << "\n";
        // if (n == write_index_) {
        //     write_index_ = 0;
        //     memset(write_buffer_, 0, n);
        //     return n;
        // }
        // memmove(write_buffer_, write_buffer_ + n, write_index_ - n);
        // return 0;
        buffer_.writeTo(fd_);
    }

    // not thread safe, must do in same thread
    void Stream::write(char *ptr, int len) {
        // MutexLock lock(mutex_);
        // if (len + write_index_ >= kBufferLength) {
        //     return -1;
        // }

        // ::strncpy(write_buffer_ + write_index_, ptr, len);
        // write_index_ += len;
        // return len;
        buffer_.append(ptr, len);
    }

    void Stream::addCallback(Event ev, Callback callback) {
        // fix me: log error information
        if (callbacks_[ev] != nullptr) {
            return ;
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
            LOG_FATAL << "error occurs during bind\n";
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

        LOG_INFO << "successfully listened on port " << port << "\n";
        setState(State::CONNECTING);
    }

    void Stream::setLastactivityAsCurrent() {
        
        last_activity_ = getCurrentTimeAsMicroscends();
    }

    long long Stream::getCurrentTimeAsMicroscends() {
        struct timeval current_time;
        int err = gettimeofday(&current_time, nullptr);
        if (err < 0) {
            return -1;
        }
        long long time_as_mirosenconds = current_time.tv_sec * 1000 + current_time.tv_usec / 1000;
        return time_as_mirosenconds;
    }

    int Stream::getExpiredTimeAsMicroscends() {
        if (getTimeout() == 0 && getTiktok() == 0) {
            return std::numeric_limits<int>::max();
        }
        if (getTimeout() != 0 && getTiktok() != 0) {
            long long mostRecentExpiredTime = std::min(getTimeout(), getTiktok());
            return mostRecentExpiredTime - (getCurrentTimeAsMicroscends() - getLastActivity());
        }
        else if (getTimeout()) {
            return getTimeout() - (getCurrentTimeAsMicroscends() - getLastActivity());
        }
        else if (getTiktok()) {
            return getTiktok() - (getCurrentTimeAsMicroscends() - getLastActivity());
        }
    }
    // note: timeout and tiktok unit is second.
    void Stream::runAfter(int timeout, Callback callback) {
        setLastactivityAsCurrent();
        setTimeout(timeout * 1000);
        addCallback(Event::TIMEOUT, callback);
    }

    void Stream::runEvery(int tiktok, Callback callback) {
        setLastactivityAsCurrent();
        setTiktok(tiktok * 1000);
        addCallback(Event::TICTOK, callback);
    }

    void Stream::close() {
        LOG_INFO << "waiting to close\n";
        setState(State::CLOSING);
    }
}// end of namespace asnet