#include "Channel.h"
#include "EventLoop.h"
#include "Conn.h"

namespace ws{
namespace net{

Channel::~Channel() {
    conn->close_connection();
    LOG_DEBUG << "channel die:" << fd_;
}

void Channel::handle_event(uint32_t mask) {
    event_handling_ = true;
    if((mask & WS_READABLE & mask_) && read_callback_){
        read_callback_();
    }
    if((mask & WS_WRITABLE & mask_) && write_callback_){
        write_callback_();
    }
    event_handling_ = false;
}

std::string Channel::to_string() {
    std::ostringstream os;

    os << "channel: [name:" << name << "] [mask:";
    if(mask_ & WS_READABLE){
        os << "R";
    }
    if(mask_ & WS_WRITABLE){
        os << "W";
    }
    os << "] [fd:" << fd() << "]";
    return os.str();
}


} // end namespace net
} // namespace ws
