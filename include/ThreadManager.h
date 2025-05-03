#ifndef THREAD_MANAGER_H
#define THREAD_MANAGER_H

#include <vector>
#include <thread>
#include <functional>
#include <queue>
#include <atomic>
#include <condition_variable>

class ThreadManager {
public:
    ThreadManager(size_t numThreads);
    ~ThreadManager();

    void start();
    void stop();
    void addTask(std::function<void()> task);
    bool isRunning() const;
    size_t getNumThreads() const;
    void setNumThreads(size_t newNumThreads);
    size_t getTaskCount() const;
    void waitForCompletion();
    size_t getActiveThreadCount() const;

private:
    void workerThread();
    void processNextTask();

    size_t numThreads;
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> taskQueue;
    std::atomic<bool> running;
    std::atomic<size_t> activeThreads;
    std::mutex queueMutex;
    std::condition_variable taskCondition;
};

#endif // THREAD_MANAGER_H
