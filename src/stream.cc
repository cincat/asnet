
#include <stream.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <limits>


namespace asnet {
    
    enum Event{
        WRITE,
        READ,
        ACCEPT,
        CONNECT,
        TIMEOUT,
        TICTOK
    };

    bool streamComp(Stream *as, Stream *bs) {
        // return (as->last_activity_ + as->timeout_) < (bs->last_activity_ + bs->timeout_);
        return as->getExpiredTimeAsMicroscends() - bs->getExpiredTimeAsMicroscends();
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

        int err = 0;
        err = ::bind(fd, (struct sockaddr *)&local, sizeof(local));
        if (err < 0) {
            // fix me:
            return ;
        }
        
        ::listen(fd, 500);
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
            return ;
        }
        ::connect(fd, (struct sockaddr*)&remote, sizeof(remote));
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
}