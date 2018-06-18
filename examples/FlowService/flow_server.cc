
#include <iostream>

#include <service.h>
#include <stream.h>
#include <log.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

class FlowServer {
public:

FlowServer(char *s) : service_(2){
    server_ = service_.newStream();
    server_->listen(2019);
    ::strncpy(file_path_, s, N);
    server_->addCallback(asnet::Event::ACCEPT, std::bind(&FlowServer::onAccept, this, std::placeholders::_1));
    server_->addCallback(asnet::Event::WRITE_COMPLETE, std::bind(&FlowServer::onWriteComplete, this, std::placeholders::_1));
    // server_->runEvery(1, std::bind(&FlowServer::onPerSecond, this, std::placeholders::_1));
    service_.start();
}

void onAccept(asnet::Stream *s) {
    int fd = ::open(file_path_, O_RDONLY);
    if (fd < 0) {
        LOG_ERROR << "open " << file_path_ << " failed: " << strerror(errno) << "\n";
    }
    s->runEvery(1, std::bind(&FlowServer::onPerSecond, this, std::placeholders::_1));
    s->setContex((void*)fd);
    int n = ::read(fd, buffer_, N);
    s->write(buffer_, n);
    flow_ += n;
}

void onWriteComplete(asnet::Stream *s) {
    void *contex = s->getContex();
    int fd = *(int *)&contex;
    int n = ::read(fd, buffer_, N);
    if (n == 0) {
        ::close(fd);
        s->close();
        return ;
    }
    s->write(buffer_, n);
    flow_ += n;
}

void onPerSecond(asnet::Stream *s) {
    struct timeval tval;
    gettimeofday(&tval, nullptr);
    struct tm *tmval = localtime(&tval.tv_sec);
    const int N = 128;
    char buffer[N] = {0};
    buffer[0] = '[';
    strftime(buffer+ 1, N, "%F %T", tmval);
    int n = strlen(buffer);
    sprintf(buffer + n, ".%ld]", tval.tv_usec);
    std::cout << buffer << " " << flow_ << " bytes/s" << std::endl;
    flow_ = 0;
}
private:
    static const int N = 1024;
    asnet::Service service_;
    asnet::Stream *server_;
    char buffer_[N];
    int flow_;
    char file_path_[N];
};

int main(int argc, char *args[]) {
    if (argc != 2) {
        std::cout << "usage: flow_server [file_path]" << std::endl;
    }

    FlowServer server(args[1]);
}