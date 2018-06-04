#include <stream.h>
#include <event_loop.h>

#include <string.h>
#include <unistd.h>

int onAccept(asnet::Connection conn) {
    char buffer[16] = "hello, world\n";
    conn.getRemote()->write(buffer, ::strlen(buffer));
    // conn.getRemote()->write("", 0);
    return strlen(buffer);
}

int main() {
    asnet::EventLoop loop;
    asnet::Stream *server = loop.newStream();
    server->listen(10086);
    server->addCallback(asnet::Event::ACCEPT, std::bind(onAccept, std::placeholders::_1));
    loop.run();
    return 0;
}