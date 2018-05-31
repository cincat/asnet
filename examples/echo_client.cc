#include <stream.h>
#include <event_loop.h>

#include <unistd.h>
#include <string.h>

void onData(asnet::Stream *s) {
    char buffer[16] = {'\0'};
    ::read(s->getFd(), buffer, 16);
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