#include <stream.h>
// #include <event_loop.h>
#include <service.h>
// #include <log.h>

#include <string.h>
#include <unistd.h>

class EchoServer {
public:
    EchoServer() :
        service_(1),
        buffer_{0} {
            server_ = service_.newStream();
            server_->listen(2018);
            server_->addCallback(asnet::Event::DATA, std::bind(&EchoServer::onData, this, std::placeholders::_1));
        }
    void start() {
        service_.start();
    }
    void onData(asnet::Stream *s) {
        while (s->readable()) {
            int n = s->read(buffer_, N);
            s->write(buffer_, n);
        }
        s->close();
    }
private:
    const static int N = 32;
    asnet::Service service_;
    asnet::Stream *server_;
    char buffer_[N];
};

int main() {
    EchoServer server;
    server.start();
    return 0;
}