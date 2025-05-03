#include "../include/ThreadManager.h"
#include <iostream>
#include <chrono>

ThreadManager::ThreadManager(size_t numThreads)
    : numThreads(numThreads), activeThreads(0) {
    if (numThreads <= 0) {
        throw std::invalid_argument("Number of threads must be positive");
    }
}

ThreadManager::~ThreadManager() {
    stop();
}

void ThreadManager::start() {
    running = true;
    activeThreads = 0;
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back(&ThreadManager::workerThread, this);
    }
    std::cout << "Started " << numThreads << " threads." << std::endl;
}

void ThreadManager::stop() {
    running = false;
    taskCondition.notify_all();
    
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threads.clear();
    std::cout << "Stopped all threads." << std::endl;
}

void ThreadManager::addTask(std::function<void()> task) {
    std::lock_guard<std::mutex> lock(queueMutex);
    taskQueue.push(task);
    taskCondition.notify_one();
}

bool ThreadManager::isRunning() const {
    return running;
}

size_t ThreadManager::getNumThreads() const {
    return numThreads;
}

void ThreadManager::setNumThreads(size_t newNumThreads) {
    if (newNumThreads <= 0) {
        throw std::invalid_argument("Number of threads must be positive");
    }
    numThreads = newNumThreads;
}

size_t ThreadManager::getTaskCount() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return taskQueue.size();
}

void ThreadManager::waitForCompletion() {
    while (getTaskCount() > 0 || activeThreads > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

size_t ThreadManager::getActiveThreadCount() const {
    return activeThreads;
}

void ThreadManager::workerThread() {
    while (running) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            taskCondition.wait(lock, [this] { return !taskQueue.empty() || !running; });
            
            if (!running) break;

            task = taskQueue.front();
            taskQueue.pop();
        }

        ++activeThreads;
        task();
        --activeThreads;
    }
}
