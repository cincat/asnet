
#include <stream.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


namespace asnet {
    bool streamComp(Stream *as, Stream *bs) {
        return (as->last_activity_ + as->time_out_) < (bs->last_activity_ + bs->time_out_);
    }

    void Stream::addCallback(Event ev, Callback callback) {
        // fix me: log error information
        if (callbacks_[ev] != nullptr) {
            return ;
        }
        callbacks_[ev] = callback;
    }

    void Stream::listen(int port) {

        int fd = socket(AF_INET, SOCK_STREAM, 0);
        // fix me: add log information
        if (fd < 0) {
            return ;
        }

        struct sockaddr_in local;
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        local.sin_port = htons(port);

        int err = 0;
        err = ::bind(fd, (struct sockaddr *)&local, sizeof(local));
        if (err < 0) {
            // fix me:
            return ;
        }
        
        ::listen(fd, 500);
        setFd(fd);
    }

    void Stream::connect(char *addr, int port) {
        int fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            return ;
        }
        setFd(fd);

        struct sockaddr_in remote;
        remote.sin_family = AF_INET;
        remote.sin_port = htons(port);
        int err = 0;
        err = inet_aton(addr, &remote.sin_addr);
        if (err < 0) {
            return ;
        }
        ::connect(fd, (struct sockaddr*)&remote, sizeof(remote));
    }
}