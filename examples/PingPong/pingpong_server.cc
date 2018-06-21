#include <stream.h>
#include <service.h>

class PingPongServer {
public:
void onData(asnet::Stream *s) {
    if (s->hasCallbackFor(asnet::Event::TIMEOUT) == false) {
        s->runAfter(10, std::bind(&PingPongServer::timeOutListener, this, std::placeholders::_1));
    }

    // if (s->hasCallbackFor(asnet::Event::TICKTOCK) == false) {
    //     s->runEvery(2, std::bind(&PingPongServer::printTime, this, std::placeholders::_1));
    // }
    // if (s->hasCallbackFor(asnet::Event::TICKTOCK) == false) {
    //     s->runEvery(1, std::bind(&PingPongServer::onPerSecond, this, std::placeholders::_1));
    // }

    int n = s->read(buffer_, N);
    // std::cout << n << std::endl;
    // std::cout << buffer_ << n << std::endl;
    s->write(buffer_, n);
    // flow_ += n;
}

void timeOutListener(asnet::Stream *s) {
    // std::cout << total_ / (9*1e6) << " Mb/s" << std::endl;
    s->close();
}

// void onPerSecond(asnet::Stream *s) {
//     struct timeval tval;
//     gettimeofday(&tval, nullptr);
//     struct tm *tmval = localtime(&tval.tv_sec);
//     const int N = 128;
//     char buffer[N] = {0};
//     buffer[0] = '[';
//     strftime(buffer+ 1, N, "%F %T", tmval);
//     int n = strlen(buffer);
//     sprintf(buffer + n, ".%ld]", tval.tv_usec);
//     std::cout << buffer << " " << flow_/1e6 << " Mb/s" << std::endl;
//     total_ += flow_;
//     flow_ = 0;
// }
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

PingPongServer() : buffer_{0}, service_(2){
    server_ = service_.newStream();
    server_->listen(2018);
    server_->addCallback(asnet::Event::DATA, std::bind(&PingPongServer::onData, this, std::placeholders::_1));
    service_.start();
}

private:
    static const int N = 16 * 1024 + 1;
    char buffer_[N];
    asnet::Service service_;
    asnet::Stream *server_;
    // int flow_;
    // long long total_;
};

int main() {
    PingPongServer server;
}