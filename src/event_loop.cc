
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <sys/eventfd.h>
// #include <unistd.h>
// #include <sys/epoll.h>

#include <event_loop.h>
#include <stream.h>
#include <log.h>
#include <service.h>

#include <limits>
#include <vector>

namespace asnet {
    bool streamComp(Stream *, Stream*);
    EventLoop::EventLoop()
        : streams_(std::bind(streamComp, std::placeholders::_1, std::placeholders::_2)),
        service_(nullptr),
        pool_(),
        repeat_(false){
        efd_ = ::epoll_create(1);
        // fix me
        if (efd_ < 0) {
            LOG_ERROR << "create event fd failed\n";
            return;
        }

        event_fd_ = ::eventfd(0, EFD_NONBLOCK);
        struct epoll_event event;
        event.events = EPOLLIN;
        event.data.ptr = nullptr;
        ::epoll_ctl(efd_, EPOLL_CTL_ADD, event_fd_, &event);
    }

    EventLoop::~EventLoop() {
        ::epoll_ctl(efd_, EPOLL_CTL_DEL, event_fd_, nullptr);
        ::close(event_fd_);
        ::close(efd_);
    }

    int EventLoop::getBlockTime() {
        Stream *stream = nullptr;
        if (streams_.empty() == false) {
            stream = *streams_.begin();
        }
        if (stream == nullptr 
            || stream->getExpireTime() == std::numeric_limits<long long>::max()) 
            return std::numeric_limits<int>::max();
        int blocktime = stream->getExpireTime() - Stream::getCurrentTimeAsMilliscends();
        if (blocktime < 0) return 0;
    }

    void EventLoop::run() {
        
        while (true) {
            registerStreamEvents();
            // long block_time = stream->getExpiredTimeAsMicroscends();
            int blocktime = getBlockTime();
            // LOG_INFO << "blocktime is " << blocktime << "\n";
            handleStreamEvents(blocktime);

            handleClosedEvents();
            if (streams_.size() + stream_buffer_.size() == 0) {
                if (repeat_ == false) break;
            }
            
            handleTimeoutEvents();

            invokeCallbacks();
        }    
    }

    // void EventLoop::stop() {
    //     quit_ = true;
    //     createEvent();

    // }
    void EventLoop::unregistStream(Stream *stream) {
        epoll_ctl(efd_, EPOLL_CTL_DEL, stream->getFd(), nullptr);
        streams_.erase(stream);
        // MutexLock lock(mutex_);
        stream_buffer_.push_back(stream);
    }

    void EventLoop::registerStreamEvents() {
        int err = 0;
        while (stream_buffer_.size() != 0) {
            auto stream = stream_buffer_.front();
            stream_buffer_.pop_front();
            if (stream->getState() == State::CONNECTING) {
                struct epoll_event event;
                event.events = EPOLLOUT;
                event.data.ptr = stream;
                err = epoll_ctl(efd_, EPOLL_CTL_ADD, stream->getFd(), &event);
                if (err < 0) {
                    LOG_ERROR << "register stream events on " << stream->getFd() << " error: " << strerror(errno) << "\n";
                }
            }
            else if (stream->getState() == State::LISTENING) {
                struct epoll_event event;
                event.events = EPOLLIN;
                event.data.ptr = stream;
                err = epoll_ctl(efd_, EPOLL_CTL_ADD, stream->getFd(), &event);
                if (err < 0) {
                    LOG_ERROR << "register stream events on " << stream->getFd() << " error: " << strerror(errno) << "\n";
                }
            }
            else if (stream->getState() == State::CONNECTED) {
                struct epoll_event event;
                event.events = EPOLLIN;
                if (stream->writable()) event.events |= EPOLLOUT;
                event.data.ptr = stream;
                err = epoll_ctl(efd_, EPOLL_CTL_ADD, stream->getFd(), &event);
                if (err < 0) {
                    LOG_ERROR << "register stream events on " << stream->getFd() << " error: " << strerror(errno) << "\n";
                }
            }
            else if (stream->getState() == State::CLOSING) {
                if (stream->writable()) {
                    struct epoll_event event;
                    event.events = EPOLLOUT;
                    event.data.ptr = stream;
                    err = epoll_ctl(efd_, EPOLL_CTL_ADD, stream->getFd(), &event);
                    if (err < 0) {
                        LOG_ERROR << "register stream events on " << stream->getFd() << " error: " << strerror(errno) << "\n";
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
        // stream_buffer_.clear();
    }

    void EventLoop::handleStreamEvents(long block_time) {
        struct epoll_event event_list[kEventNum];
        int nfds;
        do {
            nfds = epoll_wait(efd_, event_list, kEventNum, block_time);
        } while (nfds < 0 && errno == EINTR);
        // fix me: add log information
        if (nfds < 0) {
            LOG_ERROR << "epoll_wait failed: " << strerror(errno) << "\n";
            return ;
        }

        for (int i = 0; i < nfds; i++) {
            Stream *stream = static_cast<Stream *>(event_list[i].data.ptr);
            if (stream == nullptr) { //event_fd_ poll in
                LOG_INFO << "an event happend!\n";
                int err = 0;
                uint64_t n;
                err = ::read(event_fd_, &n, sizeof(uint64_t));
                if (err < 0 && errno != EAGAIN) {
                    LOG_ERROR << "read from event_fd_ failed: " << strerror(errno) << "\n";
                }
                continue ;
            }
            if (event_list[i].events & EPOLLIN) {
                
                if (stream->getState() == State::LISTENING) {
                    // fix me
                    int anofd = accept(stream->getFd(), nullptr, nullptr);
                    if (anofd < 0) {
                        LOG_ERROR << strerror(errno) << "\n";
                        return ;
                    }
                    LOG_INFO << "successfully accepted a new fd " << anofd << "\n";
                    Stream *ano_stream = newStream(anofd);
                    ano_stream->setState(State::CONNECTED);
                    // fix me further should add time callbacks
                    if (stream->hasCallbackFor(Event::DATA)) {
                        ano_stream->addCallback(Event::DATA, stream->getCallbackFor(Event::DATA));
                    }

                    if (stream->hasCallbackFor(Event::WRITE_COMPLETE)) {
                        ano_stream->addCallback(Event::WRITE_COMPLETE, stream->getCallbackFor(Event::WRITE_COMPLETE));
                    }

                    if (stream->hasCallbackFor(Event::CLOSE)) {
                        ano_stream->addCallback(Event::CLOSE, stream->getCallbackFor(Event::CLOSE));
                    }

                    if (stream->hasCallbackFor(Event::ACCEPT)) {
                        LOG_INFO << "stream has register a accept listener" << "\n";
                        Stream::Callback listener = stream->getCallbackFor(Event::ACCEPT);
                        // ano_stream->addCallback(Event::ACCEPT, stream->getCallbackFor(Event::ACCEPT));
                        // Connection conn(stream, ano_stream);
                        // ano_stream->getEventLoop()->appendCallback(std::make_pair(listener, conn));
                        // listener(conn);
                        EventLoop *loop = ano_stream->getEventLoop();
                        if (loop != nullptr) {
                            LOG_INFO << "get a loop pointer the stream belongs\n";
                            // loop->appendCallback(std::pair<Stream::Callback, Connection>(listener, conn));
                            loop->appendCallback(std::pair<Stream::Callback, Stream *>(listener, ano_stream));
                        }
                    }
                    createEvent();
                    // stream_buffer_.push_back(ano_stream);
        
                }
                else if (stream->getState() == State::CONNECTED) {
                    LOG_INFO << "data have reached\n";
                    stream->read();
                    if (stream->readable() == false) {
                        stream->close();
                        if (stream->hasCallbackFor(Event::CLOSE)) {
                            stream->getCallbackFor(Event::CLOSE)(stream);
                        }
                        continue ;
                    }
                    if (stream->hasCallbackFor(Event::DATA)) {
                        Stream::Callback callback = stream->getCallbackFor(Event::DATA);
                        callback(stream);
                        // Connection conn(stream, nullptr);
                        // int n = callback(conn);
                        // if (n == 0) {
                        //     stream->close();
                        //     unregistStream(stream);
                        //     // streams_.erase(stream);
                        //     // stream_buffer_.push_back(stream);
                        //     // epoll_ctl(efd_, EPOLL_CTL_DEL, stream->getFd(), nullptr);
                        // }
                    }
                    else {
                        LOG_ERROR << "lack data call back while data reached\n";
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
                        // Connection conn(stream, nullptr);
                        // callback(conn);
                        callback(stream);
                        // if (stream->writable() == false) {
                        //     // write complete!
                        //     if (stream->hasCallbackFor(Event::WRITE_COMPLETE)) {
                        //         Stream::Callback callback = stream->getCallbackFor(Event::WRITE_COMPLETE);
                        //         callback(stream);
                        //     }
                        // }
                    }
                    //
                    // epoll_ctl(efd_, EPOLL_CTL_DEL, stream->getFd(), nullptr);
                    // streams_.erase(stream);
                    // stream_buffer_.push_back(stream);
                    // unregistStream(stream);
                }
                else if (stream->getState() == State::CONNECTED) {
                    if (stream->writable()) {
                        stream->write();
                        if (stream->writable() == false && stream->hasCallbackFor(Event::WRITE_COMPLETE)) {
                            stream->getCallbackFor(Event::WRITE_COMPLETE)(stream);
                        }
                    }
                    
                }
                else if (stream->getState() == State::CLOSING) {
                    if (stream->writable()) {
                        stream->write();
                        if (stream->writable() == false && stream->hasCallbackFor(Event::WRITE_COMPLETE)) {
                            stream->getCallbackFor(Event::WRITE_COMPLETE)(stream);
                        }
                    }
                    // nothin left in the write buffer
                    if (stream->writable() == false) {
                        LOG_INFO << "set this stream as closed\n";
                        stream->setState(State::CLOSED);
                    }
                }
            }
            unregistStream(stream);
            // stream_buffer_.push_back(stream);
            // streams_.erase(stream);
        }
    }

    Stream *EventLoop::handleSingleTimeoutEvent(Stream *stream) {
        // std::vector<Stream *> buffer;
        if (stream->isExpired() == false) return nullptr;
            // std::cout << "here" << std::endl;
        uint64_t cur_time = Stream::getCurrentTimeAsMilliscends();
        if (stream->getTimeoutExpireTime() <= cur_time) {
            if (stream->hasCallbackFor(Event::TIMEOUT) == false) {
                LOG_ERROR << "lack of timeout callback \n";
                return nullptr ;
            }
            // std::cout << "get timeout" << std::endl;
            stream->getCallbackFor(Event::TIMEOUT)(stream);
            stream->resetTimeout();
            // buffer.push_back(stream);
            return stream;
        }

        if (stream->getTicktockExpireTime() <= cur_time) {
            if (stream->hasCallbackFor(Event::TICKTOCK) == false) {
                LOG_ERROR << "lack of ticktock callback \n";
                return nullptr ;
            }
            stream->getCallbackFor(Event::TICKTOCK)(stream);
            stream->resetTickTock();
            // buffer.push_back(stream);
            return stream;
        }
    }   

    
    void EventLoop::handleTimeoutEvents() {
        std::vector<Stream *> buffer;
        for (auto stream : streams_) {
            // std::cout << "streams_ has content" << std::endl;
            Stream *s = handleSingleTimeoutEvent(stream);
            if (s != nullptr) {
                buffer.push_back(s);
            }
        }
        for (auto stream : buffer) {
            adjustStream(stream);
        }

        buffer.clear();
        while (stream_buffer_.size()) {
            buffer.push_back(stream_buffer_.front());
            handleSingleTimeoutEvent(stream_buffer_.front());
            stream_buffer_.pop_front();
        }

        for (auto stream : buffer) {
            stream_buffer_.push_back(stream);
        }
    }
    void EventLoop::handleClosedEvents() {
        std::vector<Stream*> buffer;
        for (auto stream : streams_) {
            if (stream->getState() == State::CLOSED 
                || (stream->getState() == State::CLOSING && stream->writable() == false)) {
                LOG_INFO << "found a closed tag\n";
                buffer.push_back(stream);
            }
        }
        // LOG_INFO << "buffer_ size is " << buffer_.size() << "\n";
        for (auto stream : buffer) {
            LOG_INFO << "handling closed events\n";
            streams_.erase(stream);
            epoll_ctl(efd_, EPOLL_CTL_DEL, stream->getFd(), nullptr);
            // std::cout << stream << std::endl;
            delete stream;
            stream = nullptr;
        }
    }

    void EventLoop::createEvent() {
        uint64_t n = 1;
        int err = ::write(event_fd_, &n, sizeof(uint64_t));
        if (err < 0) {
            LOG_ERROR << strerror(errno) << "\n";
        }
         return ;
    }

    void EventLoop::appendCallback(std::pair<Stream::Callback, Stream *> closure) {
        callback_buffer_.push_back(closure);
        createEvent();
        return ;
    }

    void EventLoop::invokeCallbacks() {
        while (callback_buffer_.size() != 0) {
            std::pair<Stream::Callback, Stream *> closure = callback_buffer_.front();
            callback_buffer_.pop_front();
            closure.first(closure.second);
            // closure(this);
        }
        return ;
    }

    Stream *EventLoop::newInternalStream() {
        Stream *stream = new Stream(&pool_);
        stream->setEventLoop(this);
        stream_buffer_.push_back(stream);
        // createEvent();
        // createEvent(); // in case in sleep state
        return stream;
    }

    Stream *EventLoop::newStream(int fd) {
        if (service_ == nullptr || service_->getThreadPool()->size() == 0) {
            // Stream *stream = new Stream(&pool_, fd);
            // stream->setEventLoop(this);
            // // MutexLock lock(mutex_);
            // stream_buffer_.push_back(stream);
            // return stream;
            // return service_->getThreadPool()->newStream();
            Stream *stream = newInternalStream();
            stream->setFd(fd);
            return stream;
        }
        return service_->getThreadPool()->newStream(fd);
    }

    Stream* EventLoop::newStream() {
        return newStream(Stream::INVALID_SOCKET_FD);
    }

    // must called when stream change thier timestamp 
    void EventLoop::adjustStream(Stream *s) {
        if (streams_.empty() == false || streams_.count(s) == 0) {
            LOG_WARN << "no stream in the stream set\n";
            return ;
        }
        streams_.erase(s);
        streams_.insert(s);
    }
}