#include <service.h>
#include <stream.h>

#include <fcntl.h>

uint64_t total;

class PingPongClient {
public:
void onConnection(asnet::Stream *s) {
    // char *message = "hello, asent!";
    // s->write(message, strlen(message));
    std::string message;

    
    // char buffer[N];
    int fd = ::open("bin/pingpong.txt", O_RDONLY);
    int n = ::read(fd, buffer_, N);
    s->write(buffer_, n);
    // readn_ += n;
}
void printTime(asnet::Stream *s) {
    struct timeval tval;
    gettimeofday(&tval, nullptr);
    struct tm *tmval = localtime(&tval.tv_sec);
    const int N = 128;
    char buffer[N] = {0};
    buffer[0] = '[';
    strftime(buffer+ 1, N, "%F %T]", tmval);
    std::cout << buffer << std::endl;
}
void onData(asnet::Stream *s) {
    if (s->hasCallbackFor(asnet::Event::TIMEOUT) == false) {
        s->runAfter(10, std::bind(&PingPongClient::handleClose, this, std::placeholders::_1));
    }

    if (s->hasCallbackFor(asnet::Event::TICKTOCK) == false) {
        // s->runEvery(2, std::bind(&PingPongClient::printTime, this, std::placeholders::_1));
    }
    int n = s->read(buffer_, N);
    // readn_ += n;
    // std::cout << n << std::endl;
    total += n;
    // std::cout << buffer_ << std::endl;
    s->write(buffer_, n);
}
PingPongClient() : service_(1), buffer_{0}{
    client_ = service_.newStream();
    client_->connect("127.0.0.1", 2018);
    client_->addCallback(asnet::Event::CONNECT, std::bind(&PingPongClient::onConnection, this, std::placeholders::_1));
    client_->addCallback(asnet::Event::DATA, std::bind(&PingPongClient::onData, this, std::placeholders::_1));
    service_.start();
}

void handleClose(asnet::Stream *s) {
    s->close();
} 

// uint64_t getReadn() {return readn_;}
private:
    static const int N = 16 * 1024 + 1;
    asnet::Service service_;
    asnet::Stream *client_;
    char buffer_[N];
    uint64_t readn_;
};

int main() {
    // for (int i = 0; i < 100; i++) {
    //     PingPongClient client;
    // }
    int n = 1;
    
    struct timeval bval, eval;
    gettimeofday(&bval, nullptr);
    // struct tm *tmval = localtime(&bval.tv_sec);
    for (int i = 0; i < n; i++) {
        PingPongClient client;
    }
    gettimeofday(&eval, nullptr);

    double ans = total / (eval.tv_sec + eval.tv_usec / 1e6 - bval.tv_sec - bval.tv_usec / 1e6);
    std::cout << ans / 1e6 << " Mb/s" << std::endl;
}