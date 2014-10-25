//
// Created by dantezhu on 14-10-25.
//


#ifndef __TimerHelper_2014_10_25_H_
#define __TimerHelper_2014_10_25_H_

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
#include <Windows.h>
#else
#include <sys/time.h>
#endif

class TimerHelper {
public:
    static void getNowTime(struct timeval *tv);
    static void cmpTime(struct timeval *tv1, struct timeval *tv2);
};


#endif //__TimerHelper_2014_10_25_H_
