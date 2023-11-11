#pragma once
#include "EventLoop.h"
#include <stdbool.h>
#include "WorkerThread.h"

// 定义线程池
struct ThreadPool
{
    // 主线程的反应堆模型
    struct EventLoop* mainLoop;
    bool isStart;
    int threadNum;
    struct WorkerThread* workerThreads;
    int index;
};

// 初始化线程池
struct ThreadPool* threadPoolInit(struct EventLoop* mainLoop, int count);
// 启动线程池
void threadPoolRun(struct ThreadPool* pool);
// 取出线程池中的某个子线程的反应堆实例
struct EventLoop* takeWorkerEventLoop(struct ThreadPool* pool);