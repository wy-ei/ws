//
// Created by wangyu on 2020/6/18.
//

#include <cassert>
#include "../log/logging.h"

#include "EventLoop.h"

__thread EventLoop* this_thread_event_loop = nullptr;

EventLoop::EventLoop() :quit_(false){
    if(this_thread_event_loop){
        LOG_ERROR << "more than one event loop in this thread.";
        pthread_exit(nullptr);
    }else{
        this_thread_event_loop = this;
    }

    tid_ = std::this_thread::get_id();
    wakeup_fd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    assert(wakeup_fd_ >= 0);
    wakeup_channel_ = std::make_shared<Channel>(this, wakeup_fd_);
    wakeup_channel_->set_read_callback(std::bind(&EventLoop::handle_read, this));
    wakeup_channel_->enable_reading();
}

EventLoop::~EventLoop() {
    this_thread_event_loop = nullptr;
    wakeup_channel_->disable_all();
    wakeup_channel_->remove_self_from_loop();
    ::close(wakeup_channel_->fd());
    this_thread_event_loop = nullptr;
}

void EventLoop::loop() {
    assert_in_loop_thread();
    assert(!looping_);
    looping_ = true;

    std::vector<SP<Channel>> active_channels;
    quit_ = false;
    while (!quit_){
        active_channels = epoll_.wait(10000);
        for(auto& channel: active_channels){
            LOG_DEBUG << "epoll:" << epoll_.fd() << "  active channel:" << channel->fd();
            channel->handle_event();
        }
        run_pending_functors();
    }
    looping_ = false;
}

void EventLoop::quit() {
    quit_ = true;
    if(!is_in_loop_thread()){
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    int n = ::write(wakeup_fd_, &one, sizeof(one));
    assert(n == sizeof(one));
}

void EventLoop::handle_read(){
    uint64_t one = 1;
    ssize_t n = ::read(wakeup_fd_, &one, sizeof one);
    assert(n == sizeof(one));
}

void EventLoop::enqueue(EventLoop::Functor fn) {
    {
        std::unique_lock<std::mutex> lock(pending_functors_mutex_);
        pending_functors_.push_back(std::move(fn));
    }

    // 没有在该 loop 所在线程调用此方法，loop 线程此时可能阻塞在 epoll 上
    // 因此需要唤醒
    if(!is_in_loop_thread() || calling_pending_functors_){
        wakeup();
    }
}

bool EventLoop::is_in_loop_thread() {
    return tid_ == std::this_thread::get_id();
}

void EventLoop::run_pending_functors() {
    std::vector<Functor> functors_;
    calling_pending_functors_ = true;

    {
        std::unique_lock<std::mutex> lock(pending_functors_mutex_);
        functors_.swap(pending_functors_);
    }

    for(const Functor& fn: functors_){
        fn();
    }
    calling_pending_functors_ = false;
}

void EventLoop::run_in_loop(EventLoop::Functor fn) {
    if(is_in_loop_thread()){
        fn();
    }
    else{
        enqueue(fn);
    }
}

void EventLoop::assert_in_loop_thread() {
    assert(tid_ == std::this_thread::get_id());
}

void EventLoop::add_channel(const SP<Channel>& channel) {
    if(is_in_loop_thread()){
        epoll_.add_channel(channel);
    }else{
        run_in_loop([channel,this]{
            epoll_.add_channel(channel);
        });
    }
}

void EventLoop::update_channel(const SP<Channel> &channel) {
    add_channel(channel);
}

void EventLoop::remove_channel(const SP<Channel>& channel) {
    if(is_in_loop_thread()){
        epoll_.remove_channel(channel);
    }else{
        run_in_loop([channel,this]{
            epoll_.remove_channel(channel);
        });
    }
}






