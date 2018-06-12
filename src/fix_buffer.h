#ifndef FIX_BUFFER_H
#define FIX_BUFFER_H

#include <string>

namespace asnet {

class FixBuffer {
public:
    FixBuffer();
    void append(const std::string &);
    void writeTo(int);
    bool hasNothing();
    void reset();
    int hasRoomFor(int);
private:
    const static int kBUFFER_LENGTH = 65536;
    char buffer_[kBUFFER_LENGTH];
    int head_, tail_, left_, length_;
    
};
}

#endif