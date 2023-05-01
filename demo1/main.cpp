#include <iostream>
#include <unistd.h>
#include <cstring>
#include "threadPool.cpp"
#include "threadPool.h"

void taskFunc(void* arg)
{
    int num = *(int*)arg;
    std::cout << "thread" << std::to_string(pthread_self()) << " is working, num = " << num << std::endl;
    sleep(1);
}

int main()
{
    ThreadPool<int> pool(3, 10);
    for(int i = 0; i < 100; i++)
    {
        int* num = new int(i + 100);
        pool.addTask(Task<int>(taskFunc, num));
    }

    //检查内存泄漏
    // if (true)
    // {
    //     std::cout << "为检查内存泄漏而退出程序..." << std::endl;
    //     return 0;
    // }

    sleep(20);

    return 0;
}