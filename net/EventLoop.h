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
#include <functional>
#include <iostream>
#include <atomic>
#include <thread>
#include <mutex>

#include "poll/Poller.h"
#include "Channel.h"
#include "Timeout.h"

namespace ws{
namespace net{


extern thread_local EventLoop* thread_local_event_loop;

class EventLoop {
    using Task = std::function<void()>;
public:
    EventLoop();
    ~EventLoop();

    void start();
    void stop();

    void enqueue(Task cb);
    void run_in_loop(Task cb);

    void add_channel(std::shared_ptr<Channel> channel);
    void remove_channel(std::shared_ptr<Channel> channel);
    void update_channel(std::shared_ptr<Channel> channel);

    bool is_in_loop_thread();
    void assert_in_loop_thread();

    void wakeup() const;
private:
    void run_pending_tasks();
    void handle_timeout_channels();
    void setup_wakeup_channel();

    bool looping_ { false };
    std::atomic<bool> stop_ { false };
    std::atomic<bool> calling_pending_tasks_ {false };

    std::mutex pending_tasks_mutex_;
    std::vector<Task> pending_tasks_;

    int wakeup_fd_;
    std::shared_ptr<Channel> wakeup_channel_;

    Timeout timeout_ { 3000 };
    Poller poller_ {};

    std::unordered_map<int, std::shared_ptr<Channel>> fd_to_channel_;

    std::thread::id tid_;
};

} // end namespace net
} // namespace ws

#endif //WS_EVENTLOOP_H

