//
// Created by dantezhu on 14-10-25.
//

#include "TimerHelper.h"

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
int gettimeofday(struct timeval *tp, void *tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;

    GetLocalTime(&wtm);
    tm.tm_year     = wtm.wYear - 1900;
    tm.tm_mon     = wtm.wMonth - 1;
    tm.tm_mday     = wtm.wDay;
    tm.tm_hour     = wtm.wHour;
    tm.tm_min     = wtm.wMinute;
    tm.tm_sec     = wtm.wSecond;
    tm.tm_isdst    = -1;
    clock = mktime(&tm);
    tp->tv_sec = (long)clock;
    tp->tv_usec = wtm.wMilliseconds * 1000;

    return (0);
}

void timersub(struct timeval *endPre, struct timeval *beginPre, struct timeval *result)
{
    do
    {
        (result)->tv_sec = (endPre)->tv_sec - (beginPre)->tv_sec;
        (result)->tv_usec = (endPre)->tv_usec - (beginPre)->tv_usec;
        if ((result)->tv_usec < 0)
        {
            --(result)->tv_sec;
            (result)->tv_usec += 1000000;
        }
    }
    while (0);

}

void timeradd(struct timeval *endPre, struct timeval *beginPre, struct timeval *result)
{
    do
    {
        (result)->tv_sec = (endPre)->tv_sec + (beginPre)->tv_sec;
        (result)->tv_usec = (endPre)->tv_usec + (beginPre)->tv_usec;
        if ((result)->tv_usec >= 1000000)
        {
            ++(result)->tv_sec;
            (result)->tv_usec -= 1000000;
        }
    }
    while (0);

}

bool  timercmp(struct  timeval  *a , struct  timeval  *b , char cmp) {
    // 1 0 -1
    int result = 0;
    if (a->tv_sec > b->tv_sec) {
        result = 1;
    }
    else if (a->tv_sec < b->tv_sec) {
        result = -1;
    }
    else {
        if (a->tv_usec > b->tv_usec) {
            result = 1;
        }
        else if (a->tv_usec < b->tv_usec) {
            result = -1;
        }
        else {
            result = 0;
        }
    }

    if (cmd == '>') {
        return result == 1;
    }
    else if (cmd == '=') {
        return result == 0;
    }
    else {
        return result == -1;
    }
}

#endif
