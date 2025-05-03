#include "ThreadManager.h"

ThreadManager::ThreadManager(size_t numThreads)
    : numThreads(numThreads), running(false), activeThreads(0) {}

ThreadManager::~ThreadManager() {
    stop();
}

void ThreadManager::start() {
    running = true;
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back(&ThreadManager::workerThread, this);
    }
}

void ThreadManager::stop() {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        running = false;
    }
    taskCondition.notify_all();

    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    threads.clear();
}

void ThreadManager::addTask(std::function<void()> task) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        taskQueue.push(std::move(task));
    }
    taskCondition.notify_one();
}

void ThreadManager::waitForCompletion() {
    std::unique_lock<std::mutex> lock(queueMutex);
    completionCondition.wait(lock, [this]() {
        return taskQueue.empty() && activeThreads == 0;
    });
}

void ThreadManager::workerThread() {
    while (true) {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            taskCondition.wait(lock, [this]() {
                return !taskQueue.empty() || !running;
            });

            if (!running && taskQueue.empty())
                return;

            task = std::move(taskQueue.front());
            taskQueue.pop();
            ++activeThreads;
        }

        task(); // execute outside lock

        {
            std::lock_guard<std::mutex> lock(queueMutex);
            --activeThreads;
        }
        completionCondition.notify_all(); // Notify waiters
    }
}

bool ThreadManager::isRunning() const {
    return running;
}

size_t ThreadManager::getNumThreads() const {
    return numThreads;
}

void ThreadManager::setNumThreads(size_t newNumThreads) {
    if (!running) {
        numThreads = newNumThreads;
    }
}

size_t ThreadManager::getTaskCount() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return taskQueue.size();
}

size_t ThreadManager::getActiveThreadCount() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return activeThreads;
}
