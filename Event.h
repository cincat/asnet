#include <functional>

Class Event {
public:
    
    Event(): fd(0),
        events(0),
        onread_callback_(nullptr),
        onwrite_callback_(nullptr),
        onconnect_callback_(nullptr),
        onaccept_callback_(nullptr){

    }

    void AddListener(int event_type, CallBack call_back) {
        switch (event_type) {
            case READ_EVENT : onread_callback_ = call_back; break;
            case WRITE_EVENT : onwrite_callback = call_back; break;
            case CONNECT_EVENT : onconnect_callback_ = call_back; break;
            case ACCEPT_EVENT : onaccept_callback_ = callback; break;
        }
        return ;
    }

    void ListenOn(int port);
    void ConnecteTo(const char *);

    Event(const Event &) = delete;
    Event& operator=(Event &) = delete;
    const static int kBufferLength = 1024;
    enum {
        READ_EVENT,
        WRITE_EVENT,
        CONNECT_EVENT,
        ACCEPT_EVENT
    }
private:
    using CallBack = function<void(Event &ev)>;
    int fd_;
    int events_;
    CallBack onread_callback_;
    CallBack onwrite_callback_;
    CallBack onconnect_callback_;
    CallBack onaccept_callback_;
    CallBack buffer_[kBufferLength];
}