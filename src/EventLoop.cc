
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
// #include <sys/epoll.h>

#include <EventLoop.h>
#include <stream.h>

namespace asnet {
    enum Event;
    bool streamComp(Stream *, Stream*);
    EventLoop::EventLoop(): streams_(std::bind(streamComp, std::placeholders::_1, std::placeholders::_2)){
    }

    void EventLoop::run() {
        Stream *stream = nullptr;
        if (streams_.empty() == false) {
            stream = *streams_.begin();
        }
        if (stream == nullptr) return;
        long block_time = stream->getExpiredTimeAsMicroscends();
        int efd = ::epoll_create(1);
        // fix me
        if (efd < 0) {
            return;
        }

        while (true) {
            registerStreamEvent(efd);
            handleStreamEvent(efd, block_time);
            for (auto stream : streams_) {

                if (stream->getExpiredTimeAsMicroscends() > 0) break;

                if (stream->getTimeout() > 0 && stream->getTiktok() == 0) {
                    if (stream->hasCallbackFor(Event::TIMEOUT)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::TIMEOUT);
                        callback(stream);
                    }
                }
                else if (stream->getTimeout() == 0 && stream->getTiktok() > 0) {
                    if (stream->hasCallbackFor(Event::TICTOK)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::TICTOK);
                        // fix me: should adjust stream position in set(balance tree)
                        stream->setLastactivityAsCurrent();
                        callback(stream);
                    }
                }
                else if (stream->getTiktok() < stream->getTimeout()) {
                    if (stream->hasCallbackFor(Event::TICTOK)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::TICTOK);
                        // fix me:
                        stream->setTimeout(stream->getTimeout() - (stream->getCurrentTimeAsMicroscends() - stream->getLastActivity()));
                        stream->setLastactivityAsCurrent();
                        callback(stream);
                    }
                }
                else if (stream->getTimeout() < stream->getTiktok()) {
                    if (stream->hasCallbackFor(Event::TIMEOUT)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::TIMEOUT);
                        callback(stream);
                    }
                }
            }
        }

        
    }

    void EventLoop::registerStreamEvent(int efd) {
        for (auto stream_ptr : stream_buffer_) {
            if (stream_ptr->getState() == Stream::State::CONNECTING) {
                struct epoll_event event;
                event.events = EPOLLOUT;
                event.data.ptr = stream_ptr;
                epoll_ctl(efd, EPOLL_CTL_ADD, stream_ptr->getFd(), &event);
            }
            else if (stream_ptr->getState() == Stream::State::LISTENNING) {
                struct epoll_event event;
                event.events = EPOLLIN;
                event.data.ptr = stream_ptr;
                epoll_ctl(efd, EPOLL_CTL_ADD, stream_ptr->getFd(), &event);
            }
            else if (stream_ptr->getState() == Stream::State::CONNECTED) {
                struct epoll_event event;
                event.events = EPOLLIN;
                if (stream_ptr->writable()) event.events |= EPOLLOUT;
                event.data.ptr = stream_ptr;
                epoll_ctl(efd, EPOLL_CTL_ADD, stream_ptr->getFd(), &event);
            }
            else if (stream_ptr->getState() == Stream::State::CLOSING) {
                struct epoll_event event;
                event.events = EPOLLOUT;
                event.data.ptr = stream_ptr;
                epoll_ctl(efd, EPOLL_CTL_ADD, stream_ptr->getFd(), &event);
            }
            streams_.insert(stream_ptr);
        }
        stream_buffer_.clear();
    }

    void EventLoop::handleStreamEvent(int efd, long block_time) {
        struct epoll_event event_list[kEventNum];
        int nfds;
        nfds = epoll_wait(efd, event_list, kEventNum, block_time);
        // fix me: add log information
        if (nfds < 0) {
            return ;
        }

        for (int i = 0; i < nfds; i++) {
            Stream *stream = static_cast<Stream *>(event_list[i].data.ptr);
            if (event_list[i].events & EPOLLIN) {
                if (stream->getState() == Stream::State::LISTENNING) {
                    stream->setState(Stream::State::CONNECTED);
                    // fix me
                    int anofd = accept(stream->getFd, nullptr, nullptr);
                    if (anofd < 0) {
                        return ;
                    }
                    Stream *ano_stream = newStream(anofd);
                    ano_stream->setState(Stream::State::CONNECTED);
                    if (stream->hasCallbackFor(Event::ACCEPT)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::ACCEPT);
                        callback(ano_stream);
                    }
                }
                else if (stream->getState() == Stream::State::CONNECTED) {
                    if (stream->hasCallbackFor(Event::READ)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::READ);
                        callback(stream);
                    }
                }
            }
            else if (event_list[i].events & EPOLLOUT) {
                if (stream->getState() == Stream::State::CONNECTING) {
                    stream->setState(Stream::State::CONNECTED);
                    if (stream->hasCallbackFor(Event::CONNECT)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::CONNECT);
                        callback(stream);
                    }
                }
                else if (stream->getState == Stream::State::CONNECTED) {
                    if (stream->hasCallbackFor(Event::WRITE)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::WRITE);
                        callback(stream);
                    }
                }
            }
        }
    }

    Stream* EventLoop::newStream(int fd) {
        Stream *stream = new Stream(fd);
        stream_buffer_.push_back(stream);
        return stream;
    }

    Stream* EventLoop::newStream() {
        Stream *stream = new Stream(Stream::INVALID_SOCKET_FD);
        stream_buffer_.push_back(stream);
        return stream;
    }
}