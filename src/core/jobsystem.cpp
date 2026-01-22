#include "core/jobsystem.hpp"

JobSystem::JobSystem(int numThreads)
{
    workers.reserve(numThreads);
    running = true;

    for (int t = 0; t < numThreads; ++t) {
        workers.emplace_back([this]() {
            while (true) {
                // Worker thread main loop:
                // - Sleeps until work is available or shutdown is requested.
                // - Fetches job indices atomically.
                // - Exits inner loop when all job indices have been claimed.
                // - Terminates when 'running' becomes false
                {
                    std::unique_lock<std::mutex> lock(cvMutex);
                    cv.wait(lock, [this]() {
                        return !running.load(std::memory_order_acquire) ||
                                hasWork.load(std::memory_order_acquire);
                    });
                    if (!running.load(std::memory_order_acquire))
                        return;
                }

                while (true) {
                    int jobIndex = nextJob.fetch_add(1, std::memory_order_relaxed);
                    if (jobIndex >= totalJobs)
                        break;
                    
                    if (jobIndex < totalJobs) {
                        int begin = jobIndex * chunkSize;
                        int end = begin + chunkSize;
                        if (fn != nullptr)
                            fn(data, begin, end);
                        
                        // jobsRemaining is only used as a completion counter.
                        if (jobsRemaining.fetch_sub(1, std::memory_order_relaxed) == 1) {
                            // Last job clears hasWork to prevent workers from spinning
                            // until the next dispatch.
                            cvEnd.notify_all();
                            hasWork.store(false, std::memory_order_release);
                        }
                    }
                }
            }
        });
    }
}
JobSystem::~JobSystem() {
    running.store(false, std::memory_order_release);
    cv.notify_all();
    for (auto& woker : workers) {
        woker.join();
    }
}

void JobSystem::set_data(void* data, int blockSize, int chunkSize) {
    this->blockSize = blockSize;
    this->chunkSize = chunkSize;
    this->data = data;
    this->totalJobs = blockSize/chunkSize;
}

void JobSystem::set_executor(JobFn fn) { this->fn = fn; }

void JobSystem::dispatch() {
    nextJob = 0;
    hasWork.store(true, std::memory_order_release);
    jobsRemaining = totalJobs;
    cv.notify_all();
}

void JobSystem::wait() {
    std::unique_lock<std::mutex> lock(cvEndMutex);
    cvEnd.wait(lock, [this]() {
        return jobsRemaining.load(std::memory_order_acquire) == 0;
    });
}