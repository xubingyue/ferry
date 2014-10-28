//
// Created by dantezhu on 14-10-24.
//


#ifndef __ScriptBox_2014_10_24_H_
#define __ScriptBox_2014_10_24_H_

#include "netkit/Box.h"

namespace ferry {

class ScriptBox: public netkit::Box {
public:
    static ScriptBox* create() {
        return new ScriptBox();
    }

    int getVersion() {
        return version;
    }
    void setVersion(int _version) {
        version = _version;
    }

    int getCmd() {
        return cmd;
    }
    void setCmd(int _cmd) {
        cmd = _cmd;
    }

    int getRet() {
        return ret;
    }
    void setRet(int _ret) {
        ret = _ret;
    }

    int getSn() {
        return sn;
    }
    void setSn(int _sn) {
        sn = _sn;
    }
};

}

#endif //__ScriptBox_2014_10_24_H_
