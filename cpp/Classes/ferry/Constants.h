#ifndef __CONSTANTS_H_20141025154555__
#define __CONSTANTS_H_20141025154555__

namespace ferry {

    // 等待下次连接时间(秒)
    const int TRY_CONNECT_INTERVAL = 1;

    // 连接超时(秒)
    const int CONNECT_TIMEOUT = 10;

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
        ERROR_RECV,
    };

}

#endif /* end of include guard: __CONSTANTS_H_20141025154555__ */

