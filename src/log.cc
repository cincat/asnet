#include<log.h>

namespace asnet {

Log Log::getLog(LOG_LEVEL level) {
    return Log(level);
}

}// end of asnet