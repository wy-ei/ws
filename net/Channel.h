#ifndef WS_CHANNEL_H
#define WS_CHANNEL_H

#include <array>
#include <atomic>
#include <sstream>
#include "net/comm.h"
#include "log/logging.h"


namespace ws{
namespace net{


class Channel: public std::enable_shared_from_this<Channel>{
    using EventCallback = std::function<void()>;
public:
    explicit Channel(int fd): fd_(fd){}

    ~Channel();

    void handle_event(uint32_t mask);

    void set_read_callback(const EventCallback& callback){
        read_callback_ = callback;
    }
    void set_write_callback(const EventCallback& callback){
        write_callback_ = callback;
    }

    int fd() const { return fd_; }

    bool is_enable_timeout(){
        return enable_timeout_;
    }
    void enable_timeout(){
        enable_timeout_ = true;
    }
    void disable_timeout(){
        enable_timeout_ = false;
    }

    bool is_enable_writing() const{
        return mask_ & WS_WRITABLE;
    }
    bool is_enable_reading() const{
        return mask_ & WS_READABLE;
    }
    void enable_writing(){
        mask_ |= WS_WRITABLE;
    }
    void enable_reading(){
        mask_ |= WS_READABLE;
    }
    void disable_writing(){
        mask_ &= ~WS_WRITABLE;
    }
    void disable_reading(){
        mask_ &= ~WS_READABLE;
    }

    bool closed() const { return closed_; }
    void close() {
        closed_ = true;
    }
    std::shared_ptr<Conn> conn;
    std::string name;
    std::string to_string();
private:
    const int fd_;
    uint32_t mask_  { 0 };

    bool closed_ { false };
    std::atomic<bool> event_handling_ { false };

    EventCallback read_callback_;
    EventCallback write_callback_;
//    EventCallback close_callback_;
//    EventCallback error_callback_;

    bool enable_timeout_ { false };
};

} // end namespace net
} // namespace ws


#endif //WS_CHANNEL_H
