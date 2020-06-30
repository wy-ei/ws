//
// Created by wangyu on 2020/6/18.
//

#ifndef WS_EVENTLOOP_H
#define WS_EVENTLOOP_H

#include <vector>
#include <functional>
#include <unordered_map>
#include <string>
#include <utility>
#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>

#include "EPoll.h"
#include "Channel.h"
#include "Timeout.h"

namespace ws{
namespace net{


class EventLoop;

extern __thread EventLoop* this_thread_event_loop;

class EventLoop {
    using Functor = std::function<void()>;
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void quit();

    bool is_in_loop_thread();

    void enqueue(Functor cb);
    void run_in_loop(Functor cb);

    void add_channel(const std::shared_ptr<Channel>& channel);
    void remove_channel(const std::shared_ptr<Channel>& channel);
    void update_channel(const std::shared_ptr<Channel>& channel);

    void assert_in_loop_thread();

    void wakeup();
private:
    void handle_read();
    void run_pending_functors();
    void handle_timeout_channels();

    bool looping_ { false };
    std::atomic<bool> quit_ { false };
    std::atomic<bool> calling_pending_functors_ {false };

    std::mutex pending_functors_mutex_;
    std::vector<Functor> pending_functors_;

    int wakeup_fd_;
    std::shared_ptr<Channel> wakeup_channel_;

    Timeout timeout_ { 5000 };
    EPoll epoll_{};
    typename std::thread::id tid_;
};

} // end namespace net
} // namespace ws

#endif //WS_EVENTLOOP_H

