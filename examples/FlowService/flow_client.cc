#include <service.h>
#include <stream.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

class FlowClient {

public:

FlowClient() : service_(1) {
    client_ = service_.newStream();
    client_->connect("127.0.0.1", 2019);
    fd_ = ::open("received_file", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    client_->addCallback(asnet::Event::DATA, std::bind(&FlowClient::onData, this, std::placeholders::_1));
    // client_->addCallback(asnet::Event::TICTOK, std::bind(onPerSecond, this, std::placeholders::_1));
    service_.start();
}

void onData(asnet::Stream *s) {
    if (s->hasCallbackFor(asnet::Event::TICKTOCK) == false) {
        // client_->addCallback(asnet::Event::TICKTOCK, std::bind(&FlowClient::onPerSecond, this, std::placeholders::_1));
        s->runEvery(1, std::bind(&FlowClient::onPerSecond, this, std::placeholders::_1));
    }
    while (s->readable()) {
        int n = s->read(buffer_, N);
        if (n == 0) {
            ::close(fd_);
            s->close();
            return ;
        }
        write(fd_, buffer_, n);
        flow_ += n;
    }
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
    asnet::Service service_;
    asnet::Stream *client_;
    const static int N = 1024;
    char buffer_[N];
    int fd_;
    int flow_;
};

int main() {
    FlowClient client;
    return 0;
}