#ifdef HAVE_EPOLL

#include <unordered_map>
#include "net/poll/Poller.h"



namespace ws{
namespace net{

struct EPollState {
    int efd;
    std::unordered_map<int,uint32_t> fd_to_mask;
    int event_size = 100;
    epoll_event events[100];
};

Poller::Poller() {
    max_fd_ = -1;
    auto* state = new EPollState;
    state->efd = epoll_create(64);
    poller_state_ = state;
}


Poller::~Poller() {
    auto *state = static_cast<EPollState *>(poller_state_);
    delete state;
}

std::string Poller::poller_name() const {
    return "epoll";
}

void Poller::add_event(int fd, uint32_t mask) {
    auto *state = static_cast<EPollState *>(poller_state_);
    max_fd_ = std::max(max_fd_, fd);
    int op;

    if(state->fd_to_mask.find(fd) != state->fd_to_mask.end()){
        // merge old mask
        mask |= state->fd_to_mask[fd];
        op = EPOLL_CTL_MOD;
    }else{
        op = EPOLL_CTL_ADD;
    }
    state->fd_to_mask[fd] = mask;

    epoll_event ev {0};
    ev.data.fd = fd;

    if(mask & WS_READABLE){
        ev.events |= EPOLLIN;
    }
    if(mask & WS_WRITABLE){
        ev.events |= EPOLLOUT;
    }

    epoll_ctl(state->efd, op, fd, &ev);
}

void Poller::delete_event(int fd, uint32_t delmask) {
    auto *state = static_cast<EPollState *>(poller_state_);
    if(state->fd_to_mask.find(fd) == state->fd_to_mask.end()){
        return;
    }
    int mask = state->fd_to_mask[fd] & (~delmask);
    epoll_event ev {0};
    ev.data.fd = fd;

    if(mask & WS_READABLE){
        ev.events |= EPOLLIN;
    }
    if(mask & WS_WRITABLE){
        ev.events |= EPOLLOUT;
    }
    if(mask != WS_NONE){
        state->fd_to_mask[fd] = mask;
        epoll_ctl(state->efd, EPOLL_CTL_MOD, fd, &ev);
    }else{
        state->fd_to_mask.erase(fd);
        epoll_ctl(state->efd, EPOLL_CTL_DEL, fd, &ev);
    }
}

const std::vector<FileEvent> & Poller::poll(timeval *tv) {
    auto *state = static_cast<EPollState *>(poller_state_);

    int timeout = tv ? (tv->tv_sec * 1000 + tv->tv_usec / 1000) : -1;
    int n = epoll_wait(state->efd, state->events, state->event_size, timeout);
    fired_events_.clear();
    if(n > 0) {
        for (int i = 0; i < n; i++) {
            uint32_t mask = 0;
            epoll_event* ev = state->events + i;

            if (ev->events & EPOLLIN) mask |= WS_READABLE;
            if (ev->events & EPOLLOUT) mask |= WS_WRITABLE;
            if (ev->events & EPOLLERR) mask |= WS_READABLE;
            if (ev->events & EPOLLHUP) mask |= WS_READABLE;

            if(mask){
                fired_events_.push_back(FileEvent{ev->data.fd, mask});
            }
        }
    }
    return fired_events_;
}


} // end namespace net
} // namespace ws

#endif