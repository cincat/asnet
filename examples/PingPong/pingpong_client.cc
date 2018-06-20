#include <service.h>
#include <stream.h>

class PingPongClient {
public:
void onConnection(asnet::Stream *s) {
    char *message = "hello, asent!";
    s->write(message, strlen(message));
}
void onData(asnet::Stream *s) {
    int n = s->read(buffer_, 32);
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
private:
    asnet::Service service_;
    asnet::Stream *client_;
    char buffer_[32];
};

int main() {
    PingPongClient client;
}