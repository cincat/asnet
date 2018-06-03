#include <stream.h>
#include <event_loop.h>
#include <log.h>

#include <unistd.h>
#include <string.h>

using namespace asnet;

void onData(asnet::Event e) {
    char buffer[16] = {'\0'};
    int n = ::read(e.getLocal()->getFd(), buffer, 16);
    if (n == 0) {
        e.getLocal()->close();
    }
    LOG_INFO << "has successfully read " << n << " bytes \n";
    ::write(STDOUT_FILENO, buffer, strlen(buffer));
}

int main() {
    asnet::EventLoop loop;
    asnet::Stream *client = loop.newStream();
    client->connect("127.0.0.1", 10086);
    client->addCallback(asnet::Event::DATA, std::bind(onData, std::placeholders::_1));
    loop.run();
    return 0;
}