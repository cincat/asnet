#ifndef STREAM_H
#define STREAM_H

#include <vector>
#include <functional>

#include <unistd.h>
#include <sys/time.h>

#include <connection.h>

namespace asnet {
// class Event;

enum Event{
    DATA,
    ACCEPT,
    CONNECT,
    TIMEOUT,
    TICTOK
};

enum State{
    CLOSED,
    CLOSING,
    CONNECTING,
    CONNECTED,
    LISTENING
};

class Stream {
public:
    using Callback = std::function<void (const Connection&)>;
    const static int INVALID_SOCKET_FD = -1;




    // Stream(): state_(State::CLOSED),
    //     last_activity_(::time(NULL)),
    //     callbacks_(kEventNum){}
    Stream(int fd): fd_(fd), state_(State::CLOSED),
        // last_activity_(::time(nullptr)),
        callbacks_(kEventNum, nullptr),
        write_index_(0) {}
    Stream(): Stream(INVALID_SOCKET_FD) {
        // struct timeval cur_time;
        // ::gettimeofday(&cur_time, nullptr);
        // last_activity_ = cur_time.tv_sec * 1000 + cur_time.tv_usec / 1000;
    }

    // fix me: close Stream
    ~Stream() {
        if (fd_ != INVALID_SOCKET_FD) {
            ::close(fd_);
        }
    }

    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;

    void listen(int port);
    void connect(char* addr, int port);
    void addCallback(Event type, Callback callback);
    // return time left until next time expired event, time unit is milliseconds
    // long getExpiredTime();
    int getFd() {return fd_;}
    void setFd(int fd) {fd_ = fd;}
    State getState() {return state_;}
    void setState(State state) {state_ = state;}
    bool writable() {return write_index_ > 0;}
    // friend bool streamComp(Stream *, Stream*);
    bool hasCallbackFor(Event ev) {return callbacks_[ev] != nullptr;}
    Callback getCallbackFor(Event ev) {return callbacks_[ev];}
    void runAfter(int timeout, Callback callback);
    void runEvery(int tiktok, Callback callback);
    void setTimeout(int timeout) {timeout_ = timeout;}
    int getTimeout() {return timeout_;}
    void setTiktok(int tiktok) {tiktok_ = tiktok;}
    int getTiktok() {return tiktok_;}
    void setLastactivityAsCurrent();
    long long getLastActivity() {return last_activity_;}
    long long getCurrentTimeAsMicroscends();
    int getExpiredTimeAsMicroscends();
    int write();
    int write(char*, int);
    void close() {setState(State::CLOSING);}
private:

    const static int kBufferLength = 1024;
    const static int kEventNum = 6;
    std::vector<Callback> callbacks_;
    int fd_;
    State state_;
    // time unit is mcro second which is compatible with epoll
    long long last_activity_;
    int timeout_, tiktok_;
    char write_buffer_[kBufferLength];
    // char read_buffer_[kBufferLength];
    int write_index_;
    // int read_index_;
    std::vector<Stream *> associated_streams_;
};

}// end of the namespace asnet

#endif