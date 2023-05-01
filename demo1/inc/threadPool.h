#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include <pthread.h>
#include "task.h"
#include "task.cpp"

template<typename T>
class ThreadPool
{
public:
    ThreadPool(int min, int max);
    ~ThreadPool();

    void addTask(Task<T> task);
    int getBusyNumber();
    int getAliveNumber();

private:
    static void* worker(void*);
    static void* manager(void*);
    void threadExit();

private:
    pthread_mutex_t m_lock;    
    pthread_cond_t m_cond;
    pthread_t* m_threadIDs;
    pthread_t m_managerID;
    TaskQueue<T>* m_taskQ;
    int m_minNum;
    int m_maxNum;
    int m_busyNum;
    int m_aliveNum;
    int m_exitNum;
    bool m_shutdown = false;
};

#endif