
#include <service.h>
#include <stream.h>
#include <connection.h>

#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char *filePath = nullptr;

int onAccept(asnet::Stream *s) {
    int fd = 0;
    if (filePath != nullptr) {
        fd = ::open(filePath, O_RDONLY);
    }

    if (fd == -1) {
        return -1;
    }

    const int N = 1024;
    char buffer[N];
    while(true) {
        int n = read(fd, buffer, N);
        if (n == 0) break;
        s->write(buffer, n);
    }
    s->close();
}

int main(int argc, char *args[]) {
    if (argc != 2) {
        std::cout << "usage: transfer_server [file]" << std::endl;
        return -1;
    }
    filePath = args[1];
    asnet::Service service(2);
    asnet::Stream *server = service.newStream();
    server->listen(2018);
    server->addCallback(asnet::Event::ACCEPT, std::bind(onAccept, std::placeholders::_1));
    service.start();
    return 0;
}