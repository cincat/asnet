#include <stream.h>
#include <service.h>
#include <log.h>

#include <unistd.h>
#include <string.h>

class EchoClient {
public:
    void onConnect(asnet::Stream *s) {
        char *message = "hello, asnet!";
        s->write(message, strlen(message));
    }
    void onData(asnet::Stream *s) {
        while (s->readable()) {
            int n = s->read(buffer_, N - 1);
            buffer_[n] = 0;
            std::cout << buffer_ << std::endl;
        }
    }
    void start() {
        service_.start();
    }

    EchoClient() : service_(1), buffer_{0} {
        client_ = service_.newStream();
        client_->connect("127.0.0.1", 2018);
        client_->addCallback(asnet::Event::CONNECT, std::bind(&EchoClient::onConnect, this, std::placeholders::_1));
        client_->addCallback(asnet::Event::DATA, std::bind(&EchoClient::onData, this, std::placeholders::_1));
    }
private:
    static const int N = 32;
    asnet::Service service_;
    asnet::Stream *client_;
    char buffer_[N];
};


int main() {
    EchoClient client;
    client.start();
    return 0;
}