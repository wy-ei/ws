#include "EPoll.h"


bool EPoll::has_channel(const SP<Channel>& channel) const {
    int fd = channel->fd();
    auto it = fd_to_channel_.find(fd);
    return  it != fd_to_channel_.end() && it->second == channel;
}

void EPoll::add_channel(const SP<Channel>& channel) {
    if(has_channel(channel)){
        update(EPOLL_CTL_MOD, channel);
    }else{
        fd_to_channel_[channel->fd()] = channel;
        update(EPOLL_CTL_ADD, channel);
    }
}

void EPoll::remove_channel(const SP<Channel>& channel) {
    int fd = channel->fd();
    if(fd_to_channel_.find(fd) != fd_to_channel_.end()){
        fd_to_channel_.erase(fd);
        update(EPOLL_CTL_DEL, channel);
    }
}

const char* operation_to_string(int op)
{
    switch (op)
    {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
            return "Unknown Operation";
    }
}

std::string events_to_string(int e){
    std::string s;
    if(e & EPOLLIN){
        s += "IN ";
    }
    if(e & EPOLLOUT){
        s += "OUT ";
    }
    return s;
}

void EPoll::update(int op, const SP<Channel>& channel) {
    int fd = channel->fd();
    epoll_event ev{};
    bzero(&ev, sizeof(ev));
    ev.events = channel->events();
    ev.data.fd = fd;
    std::string event_str = events_to_string(channel->events());
    debug("EPoll %d - %s <%s> on fd:<%d>, epoll fd size: %d\n", epoll_fd_, operation_to_string(op), event_str.c_str(), channel->fd(), fd_to_channel_.size());
    int ret = ::epoll_ctl(epoll_fd_, op, fd, &ev);
    assert(ret == 0);
}

std::vector<SP<Channel>> EPoll::get_activate_channels(int n) {
    std::vector<SP<Channel>> activate_channels;
    assert(n <= events_.size());
    for(int i=0;i<n;i++){
        int fd = events_[i].data.fd;
        assert(fd_to_channel_.find(fd) != fd_to_channel_.end());
        auto channel = fd_to_channel_[fd];
        if(!channel->closed()){
            channel->set_revents(events_[i].events);
            activate_channels.push_back(channel);
        }
    }
    return activate_channels;
}

std::vector<SP<Channel>> EPoll::wait(int timeout_ms) {
    int n = epoll_wait(epoll_fd_, events_.data(), events_.size(), timeout_ms);
    if(n > 0){
        return get_activate_channels(n);
    }
    else if(n == 0){
        //debug("timeout and nothing happened\n");
    }
    else {
        if (errno != EINTR) {
            debug("epoll_wait failure: %s\n", strerror(errno));
        }
    }
    return {};
}
