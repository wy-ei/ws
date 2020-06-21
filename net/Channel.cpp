#include <sys/epoll.h>

#include "Channel.h"
#include "EventLoop.h"

Channel::Channel(EventLoop *loop, int fd): loop_(loop), fd_(fd) {
}

void Channel::handle_event() {
    event_handling_ = true;

    if((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)){
        if(close_callback_) close_callback_();
    }

    if(revents_ & EPOLLERR){
        if(error_callback_) error_callback_();
    }

    if(revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)){
        if(read_callback_) read_callback_();
    }

    if(revents_ & EPOLLOUT){
        if(write_callback_) write_callback_();
    }

    event_handling_ = false;
}

void Channel::close() {
    closed_ = true;
    // auto loop = owner_loop_.lock();
}

void Channel::update() {
    loop_->update_channel(shared_from_this());
}

void Channel::remove_self_from_loop() {
    loop_->remove_channel(shared_from_this());
}





