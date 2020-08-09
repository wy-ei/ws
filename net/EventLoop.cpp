//
// Created by wangyu on 2020/6/18.
//

#include <cassert>
#include <vector>

#include "EventLoop.h"
#include "Conn.h"
#include "../log/logging.h"



namespace ws{
namespace net{

thread_local EventLoop* thread_local_event_loop;

EventLoop::EventLoop(){
    if(thread_local_event_loop != nullptr){
        LOG_ERROR << "more than one event loop in this thread.";
        pthread_exit(nullptr);
    }else{
        thread_local_event_loop = this;
    }
    tid_ = std::this_thread::get_id();
    setup_wakeup_channel();
}

EventLoop::~EventLoop() {
    thread_local_event_loop = nullptr;
    remove_channel(wakeup_channel_);
    thread_local_event_loop = nullptr;
}

void EventLoop::start() {
    assert_in_loop_thread();
    assert(!looping_);
    looping_ = true;

    std::vector<Channel*> active_channels;
    stop_ = false;
    while (!stop_){
        timeval tv {};
        tv.tv_sec = 1;

        const std::vector<FileEvent>& events = poller_.poll(&tv);
        for(const auto& ev: events){
            auto channel = fd_to_channel_[ev.fd];
            LOG_DEBUG << channel->to_string() << " ev.mask: " << ev.mask;
            channel->handle_event(ev.mask);
            if(!channel->closed() && channel->is_enable_timeout()){
                timeout_.add(channel);
            }
        }
        run_pending_tasks();
        handle_timeout_channels();
    }
    looping_ = false;
}

void EventLoop::stop() {
    stop_ = true;
    if(!is_in_loop_thread()){
        wakeup();
    }
}

void EventLoop::setup_wakeup_channel() {
    int fds[2];
    pipe(fds);
    wakeup_fd_ = fds[1];
    // wakeup_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

    int read_fd = fds[0];
    auto func = [](int fd){
        uint64_t one = 1;
        ssize_t n = ::read(fd, &one, sizeof one);
    };
    wakeup_channel_ = std::make_shared<Channel>(read_fd);
    wakeup_channel_->set_read_callback(std::bind(func, read_fd));
    wakeup_channel_->enable_reading();
    wakeup_channel_->name = "wakeup channel";
    add_channel(wakeup_channel_);
}


void EventLoop::wakeup() const {
    uint64_t one = 1;
    int n = ::write(wakeup_fd_, &one, sizeof(one));
    assert(n == sizeof(one));
}


void EventLoop::enqueue(EventLoop::Task fn) {
    {
        std::unique_lock<std::mutex> lock(pending_tasks_mutex_);
        pending_tasks_.push_back(std::move(fn));
    }

    // 没有在该 loop 所在线程调用此方法，loop 线程此时可能阻塞在 epoll 上
    // 因此需要唤醒
    if(!is_in_loop_thread() || calling_pending_tasks_){
        wakeup();
    }
}

bool EventLoop::is_in_loop_thread() {
    return tid_ == std::this_thread::get_id();
}

void EventLoop::run_pending_tasks() {
    std::vector<Task> tasks_;
    calling_pending_tasks_ = true;
    {
        std::unique_lock<std::mutex> lock(pending_tasks_mutex_);
        tasks_.swap(pending_tasks_);
    }

    for(const Task& fn: tasks_){
        fn();
    }
    calling_pending_tasks_ = false;
}

void EventLoop::run_in_loop(EventLoop::Task fn) {
    if(is_in_loop_thread()){
        fn();
    }
    else{
        enqueue(fn);
    }
}

void EventLoop::assert_in_loop_thread() {
    assert(tid_ == thread_local_event_loop->tid_);
}

void EventLoop::add_channel(std::shared_ptr<Channel> channel) {
    assert_in_loop_thread();
    if(channel->is_enable_timeout()){
        timeout_.add(channel);
    }
    int fd = channel->fd();
    assert(fd_to_channel_.find(fd) == fd_to_channel_.end());
    fd_to_channel_[fd] = channel;

    update_channel(channel);
}

void EventLoop::update_channel(std::shared_ptr<Channel> channel) {
    assert_in_loop_thread();
    int fd = channel->fd();
    assert(fd_to_channel_.find(fd) != fd_to_channel_.end());
    if(channel->is_enable_reading()){
        poller_.add_event(fd, WS_READABLE);
    }else{
        poller_.delete_event(fd, WS_READABLE);
    }
    if(channel->is_enable_writing()){
        poller_.add_event(fd, WS_WRITABLE);
    }else{
        poller_.delete_event(fd, WS_WRITABLE);
    }
    wakeup();
}

void EventLoop::remove_channel(std::shared_ptr<Channel> channel) {
    assert_in_loop_thread();
    int fd = channel->fd();
    assert(fd_to_channel_.find(fd) != fd_to_channel_.end());

    poller_.delete_event(fd, WS_READABLE | WS_WRITABLE);

    if(channel->is_enable_timeout()){
        timeout_.remove(channel);
    }
    fd_to_channel_.erase(fd);
}

void EventLoop::handle_timeout_channels() {
    std::vector<std::shared_ptr<Channel>> channels = timeout_.get_timeout_channels();
    for(auto& channel: channels){
        LOG_WARN << "timeout channel:" << channel->to_string();
    }
    for(auto& channel: channels){
        if(!channel->closed()){
            remove_channel(channel);
        }
    }
}

} // end namespace net
} // namespace ws



