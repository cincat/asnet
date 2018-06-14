#include <service.h>
#include <stream.h>
#include <connection.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void onData(asnet::Stream *s) {
    const int N = 1024;
    char buffer[N] = {0};
    char *pathName = "received_file";
    int fd = open(pathName, O_CREAT | O_RDWR);
    while (s->readable()) {
        int n = s->read(buffer, N);
        write(fd, buffer, n);
    }
    close (fd);
    s->close();
    // s->close();
}

int main() {
    asnet::Service service(1);
    asnet::Stream *client = service.newStream();
    client->connect("127.0.0.1", 2018);
    client->addCallback(asnet::Event::DATA, std::bind(onData, std::placeholders::_1));
    service.start();
    return 0;
}