#include "../include/ThreadManager.h"
#include <stdexcept>
#include <iostream>
#include <chrono>

ThreadManager::ThreadManager(size_t numThreads)
    : numThreads(numThreads), running(false), activeThreads(0) {
    if (numThreads <= 0) {
        throw std::invalid_argument("Number of threads must be positive");
    }
}

ThreadManager::~ThreadManager() {
    stop();
}

void ThreadManager::start() {
    if (running) {
        std::cerr << "Threads are already running." << std::endl;
        return;
    }
    
    running = true;
    activeThreads = 0;

    // Launch worker threads
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back(&ThreadManager::workerThread, this);
    }

    std::cout << "Started " << numThreads << " threads." << std::endl;
}

void ThreadManager::stop() {
    if (!running) {
        std::cerr << "Threads are already stopped." << std::endl;
        return;
    }

    running = false;

    // Notify all threads to stop
    taskCondition.notify_all();

    // Join all threads
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
    // Optionally, handle thread resizing (e.g., adding or removing threads)
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

void ThreadManager::processNextTask() {
    std::lock_guard<std::mutex> lock(queueMutex);
    if (!taskQueue.empty()) {
        auto task = taskQueue.front();
        taskQueue.pop();
        task();
    }
}

void ThreadManager::workerThread() {
    while (running) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            taskCondition.wait(lock, [this] { return !taskQueue.empty() || !running; });

            if (!running) {
                break;
            }

            // Retrieve the next task
            task = taskQueue.front();
            taskQueue.pop();
        }

        // Increment the active thread count
        ++activeThreads;

        // Execute the task
        task();

        // Decrement the active thread count
        --activeThreads;
    }
}
