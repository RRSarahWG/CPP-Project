#pragma once
#include <vector>
#include <thread>
#include <functional>
#include <mutex>
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
    void processNextTask();
    void workerThread();
    
    size_t numThreads;
    std::atomic<bool> running{false};
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> taskQueue;
    std::mutex queueMutex;
    std::condition_variable taskCondition;
    size_t activeThreads;
};

