#ifndef __DELEGATE_H_20141014200158__
#define __DELEGATE_H_20141014200158__


#include "Box.h"
#include "NetService.h"

namespace ferry {

template<class T>
class Delegate
{
public:
	virtual void onOpen(NetService<T> *netService) = 0;
	virtual void onMessage(NetService<T> *netService, T *box) = 0;
	virtual void onClose(NetService<T> *netService) = 0;
};

}


#endif /* end of include guard: __DELEGATE_H_20141014200158__ */
