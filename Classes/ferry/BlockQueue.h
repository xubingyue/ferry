#ifndef BLOCK_QUEUE_H_20140728115604
#define BLOCK_QUEUE_H_20140728115604


#include <deque>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>

#if defined(_WIN32) || (defined(CC_TARGET_PLATFORM) && CC_TARGET_PLATFORM==CC_PLATFORM_WIN32)
#pragma comment(lib,"pthreadVSE2.lib")
#endif

namespace ferry {

template<class DataType>
class BlockQueue {

private:
    void _init(int maxsize) {
        m_count = 0;
        m_maxsize = maxsize;

        pthread_mutex_init(&m_not_full_mutex, NULL);
        pthread_mutex_init(&m_not_empty_mutex, NULL);
        pthread_cond_init(&m_not_full_cond, NULL);
        pthread_cond_init(&m_not_empty_cond, NULL);
    }

public:
    BlockQueue() {
        _init(0);
    }

    BlockQueue(int maxsize) {
        _init(maxsize);
    }

    virtual ~BlockQueue() {
        pthread_mutex_destroy(&m_not_full_mutex);
        pthread_mutex_destroy(&m_not_empty_mutex);
        pthread_cond_destroy(&m_not_full_cond);
        pthread_cond_destroy(&m_not_empty_cond);
    }

    void setMaxSize(int maxsize) {
        m_maxsize = maxsize;
    }

    int push(DataType &d) {
        // 默认是阻塞的

        return push(d, -1);
    }

    int push_nowait(DataType &d) {
        // 不阻塞
        return push(d, 0);
    }

    int pop(DataType &d) {
        // 阻塞
        return pop(d, -1);
    }

    int pop_nowait(DataType &d) {
        // 非阻塞
        return pop(d, 0);
    }

    // block_sec: 0: 不阻塞; <0: 永久阻塞; >0: 阻塞时间
    int push(const DataType &d, int block_sec) {

        int ret;

        pthread_mutex_lock(&m_not_full_mutex);
        while (m_maxsize > 0 && m_count >= m_maxsize) {
            if (block_sec > 0) {
                struct timespec timeout;

                timeout.tv_sec = time(NULL) + block_sec;
                timeout.tv_nsec = 0;

                // 如果要阻塞，就等在这里
                ret = pthread_cond_timedwait(&m_not_full_cond, &m_not_full_mutex, &timeout);
                if (ret == ETIMEDOUT) {
                    // 是超时了，并没有等到成功
                    pthread_mutex_unlock(&m_not_full_mutex);
                    return ret;
                }
            }
            else if (block_sec < 0) {
                pthread_cond_wait(&m_not_full_cond, &m_not_full_mutex);
            }
            else {
                // 返回复制，说明push失败
                pthread_mutex_unlock(&m_not_full_mutex);
                return -1;
            }

        }

        m_queue.push_back(d);
        m_count++;
        pthread_cond_signal(&m_not_empty_cond);
        pthread_mutex_unlock(&m_not_full_mutex);

        return 0;
    }

    int pop(DataType &d, int block_sec) {
        int ret;

        pthread_mutex_lock(&m_not_empty_mutex);
        while (m_count <= 0)
            if (block_sec > 0) {
                struct timespec timeout;

                timeout.tv_sec = time(NULL) + block_sec;
                timeout.tv_nsec = 0;
                
                ret = pthread_cond_timedwait(&m_not_empty_cond, &m_not_empty_mutex, &timeout);
                if (ret == ETIMEDOUT) {
                    // 是超时了，并没有等到成功
                    pthread_mutex_unlock(&m_not_empty_mutex);
                    return ret;
                }
            }
            else if (block_sec < 0) {
                pthread_cond_wait(&m_not_empty_cond, &m_not_empty_mutex);
            }
            else {
                pthread_mutex_unlock(&m_not_empty_mutex);
                return -1;
            }

        d = m_queue.front();
        m_queue.pop_front();
        m_count--;
        pthread_cond_signal(&m_not_full_cond);
        pthread_mutex_unlock(&m_not_empty_mutex);

        return 0;

    }

    // 不能有clear函数，否则指针都会内存泄漏
    /*
    void clear() {
        m_queue.clear();
    }
    */

    size_t size() {
        return m_queue.size();
    }


private:
    int m_count;
    int m_maxsize;
    pthread_mutex_t m_not_full_mutex;
    pthread_mutex_t m_not_empty_mutex;
    pthread_cond_t m_not_full_cond;
    pthread_cond_t m_not_empty_cond;

    std::deque<DataType> m_queue;
};

}

#endif /* end of include guard: BLOCK_QUEUE_H_20140728115604 */
