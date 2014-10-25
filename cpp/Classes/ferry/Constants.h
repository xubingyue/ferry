#ifndef __CONSTANTS_H_20141025154555__
#define __CONSTANTS_H_20141025154555__

namespace ferry {

    // 超时检查间隔
    const float TIMEOUT_CHECK_INTERVAL = 1.0;

    // 等待下次连接时间(秒)
    const int TRY_CONNECT_INTERVAL = 1;

    // 发送队列大小(-1为不限制)
    const int MSG_QUEUE_TO_SERVER_MAX_SIZE = 100;

    enum EVENT_TYPE {
        EVENT_OPEN = 1,
        EVENT_SEND,
        EVENT_RECV,
        EVENT_CLOSE,
        EVENT_ERROR,
        EVENT_TIMEOUT,
    };

    enum ERROR_CODE {
        ERROR_OPEN = 1,
        ERROR_SEND,
    };

}

#endif /* end of include guard: __CONSTANTS_H_20141025154555__ */

