#pragma once

// JobSystem execution model:
//
// - A fixed pool of worker threads is created at construction.
// - The user provides a JobFn callback operating on a user-defined data pointer.
// - set_data() defines the logical work size (blockSize) and how it is
//   partitioned into chunks (chunkSize).
// - dispatch() splits the range [0, blockSize) into jobs of size chunkSize
//   and wakes all worker threads.
// - Each worker atomically fetches a job index and processes the range
//   [jobIndex * chunkSize, jobIndex * chunkSize + chunkSize).
// - Job distribution uses atomic counters; no ordering between jobs is assumed.
// - wait() blocks the calling thread until all dispatched jobs have completed.
//
// Threading and synchronization:
//
// - Workers sleep on a condition variable when no work is available.
// - Atomic counters are used for job distribution and completion tracking.
// - The system guarantees that wait() returns only after all jobs finish.
// - The JobSystem must outlive any ongoing dispatch.

#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <condition_variable>

using JobFn = void(*)(void* data, int begin, int end);

class JobSystem {
private:
    std::vector<std::thread> workers;
    std::atomic<bool> running;
    
    std::atomic<bool> hasWork;
    std::atomic<int> nextJob;
    std::atomic<int> jobsRemaining;
    
    int blockSize;
    int chunkSize;
    int totalJobs;
    
    std::condition_variable cv;
    std::mutex cvMutex;
    
    std::condition_variable cvEnd;
    std::mutex cvEndMutex;
    
    JobFn fn;
    
    void* data;

public:
    JobSystem(int numThreads);
    ~JobSystem();
    void set_executor(JobFn fn);
    void set_data(void* data, int blockSize, int chunkSize);
    void dispatch();
    void wait();
};
