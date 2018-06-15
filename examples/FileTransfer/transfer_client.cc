#include <service.h>
#include <stream.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// void onData(asnet::Stream *s) {
//     const int N = 1024;
//     char buffer[N] = {0};
//     char *pathName = "received_file";
//     int fd = open(pathName, O_CREAT | O_RDWR);
//     while (s->readable()) {
//         int n = s->read(buffer, N);
//         write(fd, buffer, n);
//     }
//     close (fd);
//     s->close();
//     // s->close();
// }

void onData(asnet::Stream *s) {
    int fd;
    if (s->getContex() == nullptr) {
        char *pathName = "received_file";
        fd = open(pathName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        s->setContex((void*)fd);
    }
    else {
        void *contex = s->getContex();
        fd = *(int *)&contex;
    }

    // if (s->readable() == false) {
    //     // read zero bytes indicate peer closed the connection
    //     s->close();
    //     close(fd);
    //     return ;
    // }
    while (s->readable()) {
        const int N = 1024;
        char buffer[N] = {0};
        int n = s->read(buffer, N);
        write(fd, buffer, n);
    }
}

void onClose(asnet::Stream *s) {
    void *contex = s->getContex();
    int fd = *(int*)&contex;
    close(fd);
} 

int main() {
    asnet::Service service(1);
    asnet::Stream *client = service.newStream();
    client->connect("127.0.0.1", 2018);
    client->addCallback(asnet::Event::DATA, std::bind(onData, std::placeholders::_1));
    client->addCallback(asnet::Event::CLOSE, std::bind(onClose, std::placeholders::_1));
    service.start();
    return 0;
}