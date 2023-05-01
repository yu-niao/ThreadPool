#ifndef _TASK_H_
#define _TASK_H_

#include <queue>
#include <pthread.h>

using callback = void(*)(void*);

template<typename T>
struct Task
{
    Task()
    {
        function = nullptr;
        arg = nullptr;
    }

    Task(callback f, void* arg)
    {
        this->arg = (T*)arg;
        this->function = f;
    }   
    callback function;
    T* arg;
};

template<typename T>
class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();

    void addTask(Task<T> task);
    void addTask(callback func, void* arg);

    Task<T> takeTask();

    inline size_t taskNumber()
    {
        return m_queue.size();
    }

private:
    pthread_mutex_t m_mutex;
    std::queue<Task<T>> m_queue;
};

#endif