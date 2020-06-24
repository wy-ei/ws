#ifndef WS_CHANNEL_H
#define WS_CHANNEL_H

#include <array>
#include <sys/epoll.h>
#include <atomic>

#include "comm.h"
#include "../log/logging.h"


static int channel_id = 0;

class Channel: public std::enable_shared_from_this<Channel>{
    using EventCallback = std::function<void()>;
public:
public:
    Channel(EventLoop*loop, int fd);

    ~Channel(){
        LOG_DEBUG << "channel die:" << fd_;
    }

    void handle_event();

    void set_read_callback(EventCallback callback){
        read_callback_ = std::move(callback);
    }
    void set_write_callback(EventCallback callback){
        write_callback_ = std::move(callback);
    }
    void set_close_callback(EventCallback callback){
        close_callback_ = std::move(callback);
    }
    void set_error_callback(EventCallback callback){
        error_callback_ = std::move(callback);
    }

    int fd() const { return fd_; }
    int events() const { return events_; }


    void set_events(int events){ events_ = events; }
    void set_revents(int revents){ revents_ = revents; }

    bool is_writing(){ return events_ & EPOLLOUT; }
    bool is_reading(){ return events_ & EPOLLIN; }

    void enable_writing(){ events_ |= EPOLLOUT; update(); }
    void enable_reading(){ events_ |= EPOLLIN; update(); }
    void disable_writing(){ events_ &= ~EPOLLOUT; update(); }
    void disable_reading(){ events_ &= ~EPOLLIN; update(); }

    void disable_all(){ events_ = 0; update(); }

    void remove_self_from_loop();

    bool closed() const { return closed_; }
private:
    void update();

    EventLoop* loop_;
    const int fd_;
    bool closed_ { false };
    std::atomic<bool> event_handling_ { false };
    int revents_ { 0 };
    int events_ { 0 };

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;
};


#endif //WS_CHANNEL_H
