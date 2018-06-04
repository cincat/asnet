
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
// #include <sys/epoll.h>

#include <event_loop.h>
#include <stream.h>
#include <log.h>

#include <limits>

namespace asnet {
    bool streamComp(Stream *, Stream*);
    EventLoop::EventLoop(): streams_(std::bind(streamComp, std::placeholders::_1, std::placeholders::_2)){
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
        
        
        int efd = ::epoll_create(1);
        // fix me
        if (efd < 0) {
            return;
        }

        while (true) {
            registerStreamEvent(efd);
            // long block_time = stream->getExpiredTimeAsMicroscends();
            handleStreamEvent(efd, getBlockTime());
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

        
    }

    void EventLoop::registerStreamEvent(int efd) {
        for (auto stream_ptr : stream_buffer_) {
            if (stream_ptr->getState() == Stream::State::CONNECTING) {
                struct epoll_event event;
                event.events = EPOLLOUT;
                event.data.ptr = stream_ptr;
                epoll_ctl(efd, EPOLL_CTL_ADD, stream_ptr->getFd(), &event);
            }
            else if (stream_ptr->getState() == Stream::State::LISTENING) {
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
            LOG_ERROR << "epoll_wait error happend!\n";
            return ;
        }

        for (int i = 0; i < nfds; i++) {
            Stream *stream = static_cast<Stream *>(event_list[i].data.ptr);
            if (event_list[i].events & EPOLLIN) {
                if (stream->getState() == Stream::State::LISTENING) {
                    // stream->setState(Stream::State::CONNECTED);
                    // fix me
                    int anofd = accept(stream->getFd(), nullptr, nullptr);
                    if (anofd < 0) {
                        LOG_ERROR << strerror(errno) << std::endl;
                        return ;
                    }
                    LOG_INFO << "successfully accepted a new fd " << anofd << std::endl;
                    Stream *ano_stream = newStream(anofd);
                    ano_stream->setState(Stream::State::CONNECTED);
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
                else if (stream->getState() == Stream::State::CONNECTED) {
                    LOG_INFO << "data have reached\n";
                    if (stream->hasCallbackFor(Event::DATA)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::DATA);
                        Connection conn(stream, nullptr);
                        callback(conn);
                    }
                }
            }
            else if (event_list[i].events & EPOLLOUT) {
                if (stream->getState() == Stream::State::CONNECTING) {
                    int err = 0;
                    getsockopt(stream->getFd(), SOL_SOCKET, SO_ERROR, &err, nullptr);
                    if (err != 0) {
                        LOG_ERROR << strerror(err) << "\n";
                    }
                    stream->setState(Stream::State::CONNECTED);
                    if (stream->hasCallbackFor(Event::CONNECT)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::CONNECT);
                        Connection conn(stream, nullptr);
                        callback(conn);
                    }
                    //
                    epoll_ctl(efd, EPOLL_CTL_DEL, stream->getFd(), nullptr);
                    streams_.erase(stream);
                    stream_buffer_.push_back(stream);
                }
                else if (stream->getState() == Stream::State::CONNECTED) {
                    if (stream->writable()) {
                        stream->write();
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
        // Stream *stream = new Stream(Stream::INVALID_SOCKET_FD);
        // stream_buffer_.push_back(stream);
        // return stream;
        return newStream(Stream::INVALID_SOCKET_FD);
    }
}