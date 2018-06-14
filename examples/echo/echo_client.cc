#include <stream.h>
// #include <event_loop.h>
#include <service.h>
#include <log.h>

#include <unistd.h>
#include <string.h>

using namespace asnet;

void onData(asnet::Stream *s) {
    char buffer[16] = {'\0'};
    int n = s->read(buffer, 16);
    // int n = ::read(s->getFd(), buffer, 16);
    // if (n == 0) {
    //     conn.getLocal()->close();
    //     return 0;
    // }
    LOG_INFO << "has successfully read " << n << " bytes \n";
    ::write(STDOUT_FILENO, buffer, n);
    // conn.getLocal()->close();
    return ;
}

int main() {
    // asnet::EventLoop loop;
    asnet::Service service(1);
    asnet::Stream *client = service.newStream();
    client->connect("127.0.0.1", 10086);
    client->addCallback(Event::DATA, std::bind(onData, std::placeholders::_1));
    service.start();
    return 0;
}