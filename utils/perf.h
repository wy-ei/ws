//
// Created by wangyu on 2020/6/30.
//

#ifndef WS_PERF_H
#define WS_PERF_H

#include <functional>
#include <chrono>
#include <iostream>
#include <ctime>
#include <iomanip>


class RunningTime{
public:
    void start(){
        start_ = std::chrono::steady_clock::now();
        c_start_ = clock();
    }
    void stop(){
        duration_ = std::chrono::steady_clock::now() - start_;
        c_duration_ = clock() - c_start_;
    }
    void report(){
        double cpu_time = 1000.0 * c_duration_ / CLOCKS_PER_SEC;  // ms
        double wall_time = duration_.count() / 1000000.0; // ms
        std::cout << std::fixed << std::setprecision(2)
            << "CPU time: " << cpu_time << " ms\n"
            << "Wall time: " << wall_time << " ms\n";
    }
private:
    using TP = std::chrono::steady_clock::time_point;
    TP start_ {};
    TP::duration duration_ {};

    clock_t c_start_ {0};
    clock_t c_duration_ {0};
};

class ScopeRunningTime{
public:
    ScopeRunningTime(){
        rt_.start();
    }
    ~ScopeRunningTime(){
        rt_.stop();
        rt_.report();
    }
private:
    RunningTime rt_;
};

class Perf{
    using Fn = std::function<void()>;
public:
    static void measure(const Fn& fn){
        {
            ScopeRunningTime srt;
            fn();
        }
    }
};

#endif //WS_PERF_H
