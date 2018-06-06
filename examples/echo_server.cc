#include <stream.h>
// #include <event_loop.h>
#include <service.h>
// #include <log.h>

#include <string.h>
#include <unistd.h>

// using namespace asnet;
int onAccept(asnet::Connection conn) {
    char buffer[16] = "hello, world\n";
    conn.getRemote()->write(buffer, ::strlen(buffer));
    // conn.getRemote()->write("", 0);
    conn.getRemote()->close();
    return strlen(buffer);
}

int main() {
    // asnet::LOG_INFO << "step into main function\n";
    asnet::Service service(4);
    asnet::Stream *server = service.newStream();
    server->listen(10086);
    server->addCallback(asnet::Event::ACCEPT, std::bind(onAccept, std::placeholders::_1));
    service.start();
    return 0;
}