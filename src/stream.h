#include <vector>
#include <functional>

#include <time.h>

namespace asnet {

class Stream {
public:
    using Callback = std::function<void (Stream *)>;

    enum State{
        CLOSED,
        CLOSING,
        CONNECTING,
        CONNECTED,
        LISTENNING
    };
    enum Event{
        WRITE,
        READ,
        ACCEPT,
        CONNECT,
        TIMEOUT,
        TICTOK
    };

    // Stream(): state_(State::CLOSED),
    //     last_activity_(::time(NULL)),
    //     callbacks_(kEventNum){}
    Stream(int fd): fd_(fd), state_(State::CLOSED),
        last_activity_(::time(nullptr)),
        callbacks_(kEventNum) {}
    Stream(): Stream(INVALID_SOCKET_FD) {}

    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;

    void listen(int port);
    void connect(char* addr, int port);
    void addCallback(Event type, Callback callback);
    // return time left until next time expired event, time unit is milliseconds
    long getExpiredTime();
    int getFd() {return fd_;}
    State getState() {return state_;}
    void setState(State state) {state_ = state;}
    friend bool streamComp(Stream *, Stream*);
private:

    const static int kBufferLength = 1024;
    const static int kEventNum = 6;
    const static int INVALID_SOCKET_FD = -1;
    std::vector<Callback> callbacks_;
    int fd_;
    State state_;
    // long active_time_;
    long int last_activity_, time_out_, tik_tok_;
    char buffer_[kBufferLength];
    std::vector<Stream *> associated_streams_;
};

}// end of the namespace asnet