# asnet

## Introduction

A multithreaded asynchronous network library, that aims to linux platform.

## Feature

* Asynchronous
* Multi-thread
* Modern C++11 interface
* Timer
* Event-driven
* Asynchronous Log facility
* Buffer using memory pool

## Example

echo server:

```C++
class EchoServer {
public:
    EchoServer() :
        service_(1),
        buffer_{0} {
            server_ = service_.newStream();
            server_->listen(2018);
            server_->addCallback(asnet::Event::DATA, std::bind(&EchoServer::onData, this, std::placeholders::_1));
        }
    void start() {
        service_.start();
    }
    void onData(asnet::Stream *s) {
        while (s->readable()) {
            int n = s->read(buffer_, N);
            s->write(buffer_, n);
        }
        s->close();
    }
private:
    const static int N = 32;
    asnet::Service service_;
    asnet::Stream *server_;
    char buffer_[N];
};

int main() {
    EchoServer server;
    server.start();
    return 0;
}
```

echo client:

```C++
class EchoClient {
public:
    void onConnect(asnet::Stream *s) {
        char *message = "hello, asnet!";
        s->write(message, strlen(message));
    }
    void onData(asnet::Stream *s) {
        while (s->readable()) {
            int n = s->read(buffer_, N - 1);
            buffer_[n] = 0;
            std::cout << buffer_ << std::endl;
        }
    }
    void start() {
        service_.start();
    }

    EchoClient() : service_(1), buffer_{0} {
        client_ = service_.newStream();
        client_->connect("127.0.0.1", 2018);
        client_->addCallback(asnet::Event::CONNECT, std::bind(&EchoClient::onConnect, this, std::placeholders::_1));
        client_->addCallback(asnet::Event::DATA, std::bind(&EchoClient::onData, this, std::placeholders::_1));
    }
private:
    static const int N = 32;
    asnet::Service service_;
    asnet::Stream *client_;
    char buffer_[N];
};


int main() {
    EchoClient client;
    client.start();
    return 0;
}
```

Asnet provide more examples in example folder, including file transfer server and client, file transfer server with flow statistics which use timer facility, and pingpong server which is used to test throughput.