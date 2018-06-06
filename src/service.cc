
#include <service.h>

namespace asnet {
    Service::Service(int num) : threads_(num -1), loop_(){
        loop_.setService(this);
    }

    void Service::start() {
        threads_.start();
        loop_.run();
    }
}// end of asnet;