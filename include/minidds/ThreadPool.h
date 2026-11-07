// 职责：把「订阅回调」这类业务任务，从 IO 线程搬到业务线程执行，

#ifndef MINIDDS_THREADPOOL_H
#define MINIDDS_THREADPOOL_H
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
namespace minidds {

class ThreadPool{
public:
    using Task=std::function<void()>;

    ThreadPool(int numThreads);
    ~ThreadPool();

    bool enqueue(Task task);

    int threadCount() const;
    int busyCount() const;

private:
    void workerLoop();

    std::vector<std::thread> workers_;
    std::queue<Task> tasks_;

    mutable std::mutex mutex_;
    std::condition_variable cond_;
    bool stop_=false;

    int busy_=0;
    int total_=0;
    
};

}
#endif