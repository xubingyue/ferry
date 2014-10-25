//
// Created by dantezhu on 14-10-25.
//


#ifndef __Utils_2014_10_25_H_
#define __Utils_2014_10_25_H_

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
#include <Windows.h>
#else
#include <sys/time.h>
#endif

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
extern int gettimeofday(struct timeval *tp, void *tzp);
#endif

#endif //__Utils_2014_10_25_H_
