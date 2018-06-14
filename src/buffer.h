#ifndef BUFFER_H
#define BUFFER_H

namespace asnet {

class MemoryPool;

class Buffer {
public:

    Buffer(MemoryPool *);

    Buffer(const Buffer &) = delete;
    Buffer &operator=(const Buffer &) = delete;

    void append(char *, int);
    int subtract(char *, int);
    void writeTo(int);
    bool hasContent();
    void readFrom(int);
    int size();
private:
    // static const int kLENGTH = 64*1024;
    void ensureCapacity(int);

    char *buffer_;
    int head_;
    int tail_;
    int length_;
    MemoryPool *pool_;
};

}//end of asnet

#endif