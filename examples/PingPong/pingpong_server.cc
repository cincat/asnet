#include <stream.h>
#include <service.h>

class PingPongServer {
public:
void onData(asnet::Stream *s) {
    if (s->hasCallbackFor(asnet::Event::TIMEOUT) == false) {
        s->runAfter(10, std::bind(&PingPongServer::timeOutListener, this, std::placeholders::_1));
    }

    // int n = s->read(buffer_, N);
    // s->write(buffer_, n);
    s->write(s->getInBuffer());
}

void timeOutListener(asnet::Stream *s) {
    s->close();
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
};

int main() {
    PingPongServer server;
}