//
// Created by dantezhu on 14-10-15.
//


#ifndef __G_2014_10_15_H_
#define __G_2014_10_15_H_

#include <iostream>
#include "ferry.h"
#include "Box.h"
#include "EventBus.h"

class G
{
public:
    static G * instance()
    {
        static G * _instance = NULL;
        if ( _instance == NULL)
            _instance = new G();
        return _instance;
    }

    ferry::Service<netkit::Box>* getFerry() {
        return &m_ferry;
    }

    eventbus::EventBus* getEventBus() {
        return &m_eventBus;
    }


protected:
    G()
    {
        static std::auto_ptr<G> _auto_ptr = std::auto_ptr<G>(this);
    }
    G(const G&) {}
    virtual ~G(){}
    friend class std::auto_ptr<G>;

    ferry::Service<netkit::Box> m_ferry;
    eventbus::EventBus m_eventBus;
};



#endif //__G_2014_10_15_H_
