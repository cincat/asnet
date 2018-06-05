
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
// #include <sys/epoll.h>

#include <event_loop.h>
#include <stream.h>
#include <log.h>

#include <limits>
#include <vector>

namespace asnet {
    bool streamComp(Stream *, Stream*);
    EventLoop::EventLoop(): streams_(std::bind(streamComp, std::placeholders::_1, std::placeholders::_2)){
        efd_ = ::epoll_create(1);
        // fix me
        if (efd_ < 0) {
            LOG_ERROR << "create event fd failed\n";
            return;
        }
    }

    long long EventLoop::getBlockTime() {
        Stream *stream = nullptr;
        if (streams_.empty() == false) {
            stream = *streams_.begin();
        }
        if (stream == nullptr) return std::numeric_limits<int>::max();
        return stream->getExpiredTimeAsMicroscends();
    }

    void EventLoop::run() {
        
        while (true) {
            registerStreamEvents();
            // long block_time = stream->getExpiredTimeAsMicroscends();
            handleClosedEvents();
            if (streams_.size() + stream_buffer_.size() == 0) {
                break;
            }
            handleStreamEvents(getBlockTime());
            
            handleTimeoutEvents();

            
        }    
    }

    void EventLoop::registerStreamEvents() {
        int err = 0;
        for (auto stream : stream_buffer_) {
            if (stream->getState() == State::CONNECTING) {
                struct epoll_event event;
                event.events = EPOLLOUT;
                event.data.ptr = stream;
                err = epoll_ctl(efd_, EPOLL_CTL_ADD, stream->getFd(), &event);
                if (err < 0) {
                    LOG_ERROR << strerror(errno) << "\n";
                }
            }
            else if (stream->getState() == State::LISTENING) {
                struct epoll_event event;
                event.events = EPOLLIN;
                event.data.ptr = stream;
                err = epoll_ctl(efd_, EPOLL_CTL_ADD, stream->getFd(), &event);
                if (err < 0) {
                    LOG_ERROR << strerror(errno) << "\n";
                }
            }
            else if (stream->getState() == State::CONNECTED) {
                struct epoll_event event;
                event.events = EPOLLIN;
                if (stream->writable()) event.events |= EPOLLOUT;
                event.data.ptr = stream;
                err = epoll_ctl(efd_, EPOLL_CTL_ADD, stream->getFd(), &event);
                if (err < 0) {
                    LOG_ERROR << strerror(errno) << "\n";
                }
            }
            else if (stream->getState() == State::CLOSING) {
                if (stream->writable()) {
                    struct epoll_event event;
                    event.events = EPOLLOUT;
                    event.data.ptr = stream;
                    err = epoll_ctl(efd_, EPOLL_CTL_ADD, stream->getFd(), &event);
                    if (err < 0) {
                        LOG_ERROR << strerror(errno) << "\n";
                    }
                }
                else {
                    stream->setState(State::CLOSED);
                    // delete stream;
                    // stream = nullptr;
                    // continue;
                }
            }
            
            streams_.insert(stream);
        }
        stream_buffer_.clear();
    }

    void EventLoop::handleStreamEvents(long block_time) {
        struct epoll_event event_list[kEventNum];
        int nfds;
        nfds = epoll_wait(efd_, event_list, kEventNum, block_time);
        // fix me: add log information
        if (nfds < 0) {
            LOG_ERROR << "epoll_wait error happend!\n";
            return ;
        }

        for (int i = 0; i < nfds; i++) {
            Stream *stream = static_cast<Stream *>(event_list[i].data.ptr);
            if (event_list[i].events & EPOLLIN) {
                if (stream->getState() == State::LISTENING) {
                    // fix me
                    int anofd = accept(stream->getFd(), nullptr, nullptr);
                    if (anofd < 0) {
                        LOG_ERROR << strerror(errno) << std::endl;
                        return ;
                    }
                    LOG_INFO << "successfully accepted a new fd " << anofd << std::endl;
                    Stream *ano_stream = newStream(anofd);
                    ano_stream->setState(State::CONNECTED);
                    // fix me : should add associated streamscd 

                    if (stream->hasCallbackFor(Event::ACCEPT)) {
                        LOG_INFO << "stream has register a accept listener" << "\n";
                        Stream::Callback listener = stream->getCallbackFor(Event::ACCEPT);
                        // ano_stream->addCallback(Event::ACCEPT, stream->getCallbackFor(Event::ACCEPT));
                        Connection conn(stream, ano_stream);
                        listener(conn);
                    }
                    // stream_buffer_.push_back(ano_stream);
        
                }
                else if (stream->getState() == State::CONNECTED) {
                    LOG_INFO << "data have reached\n";
                    if (stream->hasCallbackFor(Event::DATA)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::DATA);
                        Connection conn(stream, nullptr);
                        int n = callback(conn);
                        if (n == 0) {
                            stream->close();
                            streams_.erase(stream);
                            stream_buffer_.push_back(stream);
                            epoll_ctl(efd_, EPOLL_CTL_DEL, stream->getFd(), nullptr);
                        }
                    }
                }
            }
            else if (event_list[i].events & EPOLLOUT) {
                if (stream->getState() == State::CONNECTING) {
                    int err = 0;
                    getsockopt(stream->getFd(), SOL_SOCKET, SO_ERROR, &err, nullptr);
                    if (err != 0) {
                        LOG_ERROR << strerror(err) << "\n";
                    }
                    stream->setState(State::CONNECTED);
                    if (stream->hasCallbackFor(Event::CONNECT)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::CONNECT);
                        Connection conn(stream, nullptr);
                        callback(conn);
                    }
                    //
                    epoll_ctl(efd_, EPOLL_CTL_DEL, stream->getFd(), nullptr);
                    streams_.erase(stream);
                    stream_buffer_.push_back(stream);
                }
                else if (stream->getState() == State::CONNECTED) {
                    if (stream->writable()) {
                        stream->write();
                    }
                }
                else if (stream->getState() == State::CLOSING) {
                    if (stream->writable()) {
                        stream->write();
                    }
                    // nothin left in the write buffer
                    if (stream->writable() == false) {
                        LOG_INFO << "set this stream as closed\n";
                        stream->setState(State::CLOSED);
                    }
                }
            }
        }
    }

    // fix me : should reset lastactivity member
    void EventLoop::handleTimeoutEvents() {
        for (auto stream : streams_) {

            if (stream->getExpiredTimeAsMicroscends() > 0) break;

            if (stream->getTimeout() > 0 && stream->getTiktok() == 0) {
                if (stream->hasCallbackFor(Event::TIMEOUT)) {
                    Stream::Callback callback = stream->getCallbackFor(Event::TIMEOUT);
                    Connection conn(stream, nullptr);
                    callback(conn);
                }
            }
            else if (stream->getTimeout() == 0 && stream->getTiktok() > 0) {
                if (stream->hasCallbackFor(Event::TICTOK)) {
                    Stream::Callback callback = stream->getCallbackFor(Event::TICTOK);
                    // fix me: should adjust stream position in set(balance tree)
                    stream->setLastactivityAsCurrent();
                    Connection conn(stream, nullptr);
                    callback(conn);
                    // callback(stream);
                }
            }
            else if (stream->getTiktok() < stream->getTimeout()) {
                if (stream->hasCallbackFor(Event::TICTOK)) {
                    Stream::Callback callback = stream->getCallbackFor(Event::TICTOK);
                    // fix me:
                    stream->setTimeout(stream->getTimeout() - (stream->getCurrentTimeAsMicroscends() - stream->getLastActivity()));
                    stream->setLastactivityAsCurrent();
                    // callback(stream);
                    Connection conn(stream, nullptr);
                    callback(conn);
                }
            }
            else if (stream->getTimeout() < stream->getTiktok()) {
                if (stream->hasCallbackFor(Event::TIMEOUT)) {
                    Stream::Callback callback = stream->getCallbackFor(Event::TIMEOUT);
                    // callback(stream);
                    Connection conn(stream, nullptr);
                    callback(conn);
                }
            }
        }
    }
    void EventLoop::handleClosedEvents() {
        std::vector<Stream*> buffer_;
        for (auto stream : streams_) {
            if (stream->getState() == State::CLOSED) {
                LOG_INFO << "found a closed tag\n";
                buffer_.push_back(stream);
            }
        }
        // LOG_INFO << "buffer_ size is " << buffer_.size() << "\n";
        for (auto stream : buffer_) {
            LOG_INFO << "handling closed events\n";
            streams_.erase(stream);
            epoll_ctl(efd_, EPOLL_CTL_DEL, stream->getFd(), nullptr);
            delete stream;
            stream = nullptr;
        }
    }
    Stream* EventLoop::newStream(int fd) {
        Stream *stream = new Stream(fd);
        stream_buffer_.push_back(stream);
        return stream;
    }

    Stream* EventLoop::newStream() {
        return newStream(Stream::INVALID_SOCKET_FD);
    }
}