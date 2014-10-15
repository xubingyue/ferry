//
// Created by dantezhu on 14-10-14.
//


#ifndef __Message_2014_10_14_H_
#define __Message_2014_10_14_H_

#include <iostream>
#include <map>

template <class BoxType>
class Message {
public:
    Message() {
        what = 0;
        box = nullptr;
    }

public:
    int what;
    BoxType *box;
};


#endif //__Message_2014_10_14_H_
