#include <iostream>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include "threadPool.h"

template<typename T>
ThreadPool<T>::ThreadPool(int min, int max)
{
    m_taskQ = new TaskQueue<T>;
    do {
    m_minNum = min;
    m_maxNum = max;
    m_aliveNum = min;
    m_busyNum = 0;

    m_threadIDs = new pthread_t[max];
    if (m_threadIDs == nullptr)
    {
        delete m_taskQ;
        std::cout << "malloc thread_t[] failed..." << std::endl;
        break;
    }

    memset(m_threadIDs, 0, sizeof(pthread_t) * max);

    if (pthread_mutex_init(&m_lock, NULL) != 0 ||
    pthread_cond_init(&m_cond, NULL) != 0)
    {
        std::cout << "init mutex or cond failed..." << std::endl;
        delete m_taskQ;
        delete []m_threadIDs;
        break;
    }

    for (int i = 0; i < min; i++)
    {
        pthread_create(&m_threadIDs[i], NULL, worker, this);
        std::cout << "the thread " << std::to_string(pthread_self()) << " is created..." << std::endl;

    }   
    pthread_create(&m_managerID, NULL, manager, this);
    }while (0);
    
}

template<typename T>
ThreadPool<T>::~ThreadPool()
{
    m_shutdown = 1;
    pthread_join(m_managerID, NULL);

    for (int i = 0; i < m_aliveNum; i++)
    {
        pthread_cond_signal(&m_cond);
    }

    if (m_taskQ)
        delete m_taskQ;
    if (m_threadIDs)
        delete []m_threadIDs;
    
    pthread_mutex_destroy(&m_lock);
    pthread_cond_destroy(&m_cond);

}

template<typename T>
void ThreadPool<T>::addTask(Task<T> task)
{
    if (m_shutdown)
        return;
    
    m_taskQ->addTask(task);

    pthread_cond_signal(&m_cond);
}

template<typename T>
int ThreadPool<T>::getAliveNumber()
{
    int threadNum = 0;
    pthread_mutex_lock(&m_lock);
    threadNum = m_aliveNum;
    pthread_mutex_unlock(&m_lock);
    return threadNum;
}

template<typename T>
int ThreadPool<T>::getBusyNumber()
{
    int busyNum = 0;
    pthread_mutex_lock(&m_lock);
    busyNum = m_busyNum;
    pthread_mutex_unlock(&m_lock);
    return busyNum;
}

template<typename T>
void* ThreadPool<T>::worker(void* arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);

    while(1)
    {
        pthread_mutex_lock(&pool->m_lock);

        while(pool->m_taskQ->taskNumber() == 0 && !pool->m_shutdown)
        {
            std::cout << "thread" << std::to_string(pthread_self()) << " waiting..." << std::endl;
            pthread_cond_wait(&pool->m_cond, &pool->m_lock);

            if (pool->m_exitNum > 0)
            {
                pool->m_exitNum--;
                if (pool->m_aliveNum > pool->m_minNum)
                {
                    pool->m_aliveNum--;
                    pthread_mutex_unlock(&pool->m_lock);
                    pool->threadExit();
                }
            }
        }

        if (pool->m_shutdown)
        {
            pthread_mutex_unlock(&pool->m_lock);
            pool->threadExit();
        }

        Task<T> task = pool->m_taskQ->takeTask();
        pool->m_busyNum++;
        pthread_mutex_unlock(&pool->m_lock);

        std::cout << "thread" << std::to_string(pthread_self()) << " start working..." << std::endl;
        task.function(task.arg);
        delete task.arg;
        task.arg = nullptr;

        std::cout << "thread" << std::to_string(pthread_self()) << " end working..." << std::endl;
        pthread_mutex_lock(&pool->m_lock);
        pool->m_busyNum--;
        pthread_mutex_unlock(&pool->m_lock);
    }

    return nullptr;
}

template<typename T>
void* ThreadPool<T>::manager(void* arg)
{
    ThreadPool* pool = static_cast<ThreadPool*>(arg);

    while (!pool->m_shutdown)
    {
        sleep(5);

        pthread_mutex_lock(&pool->m_lock);
        int queueSize = pool->m_taskQ->taskNumber();
        int liveNum = pool->m_aliveNum;
        int nusyNum = pool->m_busyNum;
        pthread_mutex_unlock(&pool->m_lock);

        const int NUMBER = 2;

        if (queueSize > liveNum && liveNum < pool->m_maxNum)
        {
            pthread_mutex_lock(&pool->m_lock);
            int num = 0;
            for (int i = 0; i < pool->m_maxNum && num < NUMBER && 
            pool->m_aliveNum < pool->m_maxNum; i++)
            {
                if (pool->m_threadIDs[i] == 0)
                {
                    pthread_create(&pool->m_threadIDs[i], NULL, worker, pool);
                    num++;
                    pool->m_aliveNum++;
                }
            }
            pthread_mutex_unlock(&pool->m_lock);
        }

        if (nusyNum * 2 < liveNum && liveNum > pool->m_minNum)
        {
            pthread_mutex_lock(&pool->m_lock);
            pool->m_exitNum = NUMBER;
            pthread_mutex_unlock(&pool->m_lock);
            for (int i = 0; i < NUMBER; i++)
            {
                pthread_cond_signal(&pool->m_cond);
            }
        }
    }

    return nullptr;
}

template<typename T>
void ThreadPool<T>::threadExit()
{
    pthread_t tid = pthread_self();
    for (int i = 0; i < m_maxNum; i++)
    {
        if (m_threadIDs[i] == tid)
        {
            std::cout << "thread" << std::to_string(pthread_self()) << " exiting..." << std::endl;
            m_threadIDs[i] = 0;
            break;
        }
        pthread_exit(NULL);
    }
}