#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

class ThreadManager {
public:
    ThreadManager(size_t numThreads);
    ~ThreadManager();

    void start();
    void stop();
    void addTask(std::function<void()> task);
    void waitForCompletion();

    bool isRunning() const;
    size_t getNumThreads() const;
    void setNumThreads(size_t newNumThreads);
    size_t getTaskCount() const;
    size_t getActiveThreadCount() const;

private:
    void workerThread();

    size_t numThreads;
    bool running;

    std::vector<std::thread> threads;
    std::queue<std::function<void()>> taskQueue;

    mutable std::mutex queueMutex;
    std::condition_variable taskCondition;
    std::condition_variable completionCondition;

    size_t activeThreads;
};

#endif // THREAD_MANAGER_H
