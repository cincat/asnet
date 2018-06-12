#include<log.h>

#include <time.h>

extern char *__progname;

namespace asnet {

Log::Log(LOG_LEVEL level) {

    // time_t cur_time = time(nullptr);
    struct timeval tval;
    gettimeofday(&tval, nullptr);
    struct tm *tmval = localtime(&tval.tv_sec);
    const int N = 128;
    char buffer[N] = {0};
    buffer[0] = '[';
    strftime(buffer+ 1, N, "%F %T", tmval);
    int n = strlen(buffer);
    sprintf(buffer + n, ".%ld][", tval.tv_usec);
    strcat(buffer, __progname);
    switch(level) {
        case INFO : strcat(buffer, "][INFO]"); break;
        case ERROR : strcat(buffer, "][ERROR]"); break;
        // case FATAL : strcat(buffer, "][FATAL]"); break;
    }
    item_.append(buffer);
    // switch(level) {
    //     case INFO: ::strcpy(prefix, "[INFO]: "); break;
    //     case ERROR: ::strcpy(prefix, "[ERROR]: "); break;
    //     // case DEBUG: ::strcpy(prefix, "[DEBUG]: "); break;
    // }
    // strcat(prefix, __progname);
    // strcat(prefix, " ");
}

}// end of asnet