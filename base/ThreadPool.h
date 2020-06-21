//
// Created by wangyu on 2020/6/13.
//

#ifndef WS_THREADPOOL_H
#define WS_THREADPOOL_H

#include "../net/comm.h"
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <list>


class ThreadPool{
public:
    explicit ThreadPool(size_t n){
        for(size_t i=0;i<n;i++){
            threads_.emplace_back(worker(*this));
        }
    }

    ThreadPool(const ThreadPool&) = delete;
    ~ThreadPool() = default;

    void enqueue(const std::function<void()>& fn){
        std::unique_lock<std::mutex> lock(mutex_);
        jobs_.push_back(fn);
        cond_.notify_one();
    }

    void shutdown(){
        {
            std::unique_lock<std::mutex> lock(mutex_);
            shutdown_ = true;
        }

        cond_.notify_all();

        for(auto& t: threads_){
            t.join();
        }
    }
private:
    class worker{
    public:
        explicit worker(ThreadPool &pool):pool_(pool){
        }

        void operator()(){
            while(true){
                std::function<void()> fn;
                {
                    std::unique_lock<std::mutex> lock(pool_.mutex_);
                    auto pred = [&]{
                        return !pool_.jobs_.empty() || pool_.shutdown_;
                    };
                    pool_.cond_.wait(lock, pred);

                    if(pool_.shutdown_ && pool_.jobs_.empty()){
                        break;
                    }

                    fn = pool_.jobs_.front();
                    pool_.jobs_.pop_front();
                }

                fn();
            }
        }
    private:
        ThreadPool& pool_;
    };

    std::vector<std::thread> threads_;
    std::list<std::function<void()>> jobs_;

    bool shutdown_ = false;

    std::condition_variable cond_;
    std::mutex mutex_;
};


#endif //WS_THREADPOOL_H
