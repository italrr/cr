#include <stdio.h>
#include <stdarg.h>
#include <ctime>
#include <mutex>

#include "Log.hpp"

static std::mutex mutex;

void __CR_console_add_line(const std::string &line);

static const unsigned lineBufferSize = 1024*4;
static char lineBuffer[lineBufferSize];

static inline std::string getTimeStamp(){
    time_t currentTime;
    struct tm *localTime;
    time(&currentTime);
    localTime = localtime(&currentTime);
    std::string hour = std::to_string(localTime->tm_hour);
    std::string minute = std::to_string(localTime->tm_min);
    std::string secs = std::to_string(localTime->tm_sec);
    if(hour.length() == 1){
        hour = "0"+hour;
    }
    if(minute.length() == 1){
        minute = "0"+minute;
    }
    if(secs.length() == 1){
        secs = "0"+secs;
    }        
    return hour+":"+minute+":"+secs;
}

bool CR::log(const std::string &_format, ...){
    mutex.lock();
    std::string format = "["+getTimeStamp()+"] "+_format;
    va_list arg;
    bool done;
    va_start (arg, _format);
    done = vsprintf(lineBuffer, format.c_str(), arg);
    printf("%s", lineBuffer);
    __CR_console_add_line(lineBuffer);
    va_end (arg);
    mutex.unlock();
    return done;
}