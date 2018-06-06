#include <stream.h>
// #include <event_loop.h>
#include <service.h>
#include <log.h>

#include <unistd.h>
#include <string.h>

using namespace asnet;

int onData(asnet::Connection conn) {
    char buffer[16] = {'\0'};
    int n = ::read(conn.getLocal()->getFd(), buffer, 16);
    // if (n == 0) {
    //     conn.getLocal()->close();
    //     return 0;
    // }
    LOG_INFO << "has successfully read " << n << " bytes \n";
    ::write(STDOUT_FILENO, buffer, strlen(buffer));
    // conn.getLocal()->close();
    return n;
}

int main() {
    // asnet::EventLoop loop;
    asnet::Service service(2);
    asnet::Stream *client = service.newStream();
    client->connect("127.0.0.1", 10086);
    client->addCallback(Event::DATA, std::bind(onData, std::placeholders::_1));
    service.start();
    return 0;
}