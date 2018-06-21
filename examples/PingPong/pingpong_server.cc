#include <stream.h>
#include <service.h>

class PingPongServer {
public:
void onData(asnet::Stream *s) {
    if (s->hasCallbackFor(asnet::Event::TIMEOUT) == false) {
        s->runAfter(10, std::bind(&PingPongServer::timeOutListener, this, std::placeholders::_1));
    }

    if (s->hasCallbackFor(asnet::Event::TICKTOCK) == false) {
        s->runEvery(1, std::bind(&PingPongServer::onPerSecond, this, std::placeholders::_1));
    }

    int n = s->read(buffer_, 32);
    // std::cout << buffer_ << n << std::endl;
    s->write(buffer_, n);
    flow_ += n;
}

void timeOutListener(asnet::Stream *s) {
    std::cout << total_ / 10 << std::endl;
    s->close();
}

void onPerSecond(asnet::Stream *s) {
    struct timeval tval;
    gettimeofday(&tval, nullptr);
    struct tm *tmval = localtime(&tval.tv_sec);
    const int N = 128;
    char buffer[N] = {0};
    buffer[0] = '[';
    strftime(buffer+ 1, N, "%F %T", tmval);
    int n = strlen(buffer);
    sprintf(buffer + n, ".%ld]", tval.tv_usec);
    std::cout << buffer << " " << flow_ << " bytes/s" << std::endl;
    total_ += flow_;
    flow_ = 0;
}

PingPongServer() : buffer_{0}, service_(2), flow_(0), total_(0){
    server_ = service_.newStream();
    server_->listen(2018);
    server_->addCallback(asnet::Event::DATA, std::bind(&PingPongServer::onData, this, std::placeholders::_1));
    service_.start();
}

private:
    char buffer_[32];
    asnet::Service service_;
    asnet::Stream *server_;
    int flow_, total_;
};

int main() {
    PingPongServer server;
}