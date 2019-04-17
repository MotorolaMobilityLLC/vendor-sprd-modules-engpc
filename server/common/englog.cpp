#include <stdio.h>
#include <stdlib.h>
#include <log/log.h>

#include "englog.h"

EngLog::EngLog(){

}

EngLog::~EngLog(){

}

#define LOG_BUF_SIZE 1024
#define TAG "ENGPC"

int EngLog::info(const char* fmt, ...){
    va_list ap;
    char buf[LOG_BUF_SIZE];
    time_t tm = {0};
    
    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    printf("%s\r\n", buf);
    return __android_log_print(ANDROID_LOG_INFO, TAG, "%s\r\n", buf);
}

int EngLog::debug(const char* fmt, ...){
    va_list ap;
    char buf[LOG_BUF_SIZE];
    time_t tm = {0};
    
    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    printf("%s\r\n", buf);
    return __android_log_print(ANDROID_LOG_DEBUG, TAG, "%s\r\n", buf);
}

int EngLog::warn(const char* fmt, ...){
    va_list ap;
    char buf[LOG_BUF_SIZE];
    time_t tm = {0};
    
    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    printf("%s\r\n", buf);
    return __android_log_print(ANDROID_LOG_WARN, TAG, "%s\r\n", buf);
}

int EngLog::error(const char* fmt, ...){
    va_list ap;
    char buf[LOG_BUF_SIZE];
    time_t tm = {0};
    
    va_start(ap, fmt);
    vsnprintf(buf, LOG_BUF_SIZE, fmt, ap);
    va_end(ap);

    printf("%s\r\n", buf);
    return __android_log_print(ANDROID_LOG_ERROR, TAG, "%s\r\n", buf);
}

