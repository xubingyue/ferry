#ifndef __DELEGATE_H_20141014200158__
#define __DELEGATE_H_20141014200158__


#include "netkit/IBox.h"
#include "Service.h"

namespace ferry {

// 传入的box，delegate要负责清空
class Delegate
{
public:
    virtual ~Delegate() {}

    virtual void onOpen(Service *service) = 0;
    // 发送成功调用，传入发送成功的box
    virtual void onSend(Service *service, netkit::IBox *ibox) = 0;
    // 接收成功调用，传入接收成功的box
    virtual void onRecv(Service *service, netkit::IBox *ibox) = 0;
    virtual void onClose(Service *service) = 0;
    // 出错调用，传入发送失败的box or null
    virtual void onError(Service *service, int code, netkit::IBox *ibox) = 0;

    virtual netkit::IBox* createBox() = 0;
    virtual void releaseBox(netkit::IBox* ibox)=0;
};

}


#endif /* end of include guard: __DELEGATE_H_20141014200158__ */
