#ifndef __DELEGATE_H_20141014200158__
#define __DELEGATE_H_20141014200158__


#include "Box.h"
#include "Service.h"

namespace ferry {

template<class T>
class Delegate
{
public:
	virtual void onOpen(Service<T> *service) = 0;
	virtual void onMessage(Service<T> *service, T *box) = 0;
	virtual void onClose(Service<T> *service) = 0;
};

}


#endif /* end of include guard: __DELEGATE_H_20141014200158__ */
