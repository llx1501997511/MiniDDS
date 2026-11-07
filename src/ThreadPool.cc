#include "minidds/ThreadPool.h"

namespace minidds{

ThreadPool::ThreadPool(int numThreads):total_(numThreads){
    workers_.reserve(numThreads);
    for(int i=0;i<numThreads;i++){
        workers_.emplace_back([this](){workerLoop();});
    }
}

ThreadPool::~ThreadPool(){
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_=true;
    }
    cond_.notify_all();
    for(auto& t:workers_){
        if(t.joinable())    t.join();
    }
}

bool ThreadPool::enqueue(Task task){
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if(stop_) return false;
        tasks_.push(std::move(task));
    }
    cond_.notify_one();
    return true;
}

int ThreadPool::threadCount() const{
    return total_;
}

int ThreadPool::busyCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return busy_;
}

void ThreadPool::workerLoop(){
    while(true){
        Task task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock,[this](){return stop_||!tasks_.empty();});
            if(stop_&&tasks_.empty()){
                return;
            }
            task=std::move(tasks_.front());
            tasks_.pop();
            busy_++;
        }
        task();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            busy_--;
        }
    }
}

}