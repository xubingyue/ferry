#ifndef __DELEGATE_H_20141014200158__
#define __DELEGATE_H_20141014200158__


#include "IBox.h"
#include "Service.h"

namespace ferry {

class Delegate
{
public:
    virtual ~Delegate() {}

    virtual void onOpen(Service *service) = 0;
    // 主要是打印log
    virtual void onSend(Service *service, netkit::IBox *ibox) = 0;
    virtual void onRecv(Service *service, netkit::IBox *ibox) = 0;
    virtual void onClose(Service *service) = 0;
    virtual void onError(Service *service, int code) = 0;

    virtual netkit::IBox* createBox() = 0;
};

}


#endif /* end of include guard: __DELEGATE_H_20141014200158__ */
