#include <stream.h>
// #include <event_loop.h>
#include <service.h>
// #include <log.h>

#include <string.h>
#include <unistd.h>

// using namespace asnet;
void onAccept(asnet::Stream *s) {
    char buffer[16] = "hello, world\n";
    s->write(buffer, ::strlen(buffer));
    // conn.getRemote()->write("", 0);
    s->close();
    return ;
}

int main() {
    // asnet::LOG_INFO << "step into main function\n";
    asnet::Service service(2);
    asnet::Stream *server = service.newStream();
    server->listen(10086);
    server->addCallback(asnet::Event::ACCEPT, std::bind(onAccept, std::placeholders::_1));
    service.start();
    return 0;
}