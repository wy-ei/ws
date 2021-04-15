#ifndef HAVE_EPOLL

#include <sys/select.h>
#include <algorithm>
#include "net/poll/Poller.h"

namespace ws{
namespace net{

struct SelectState {
    fd_set read_fds;
    fd_set write_fds;
};

Poller::Poller() {
    max_fd_ = -1;
    auto* state = new SelectState;
    FD_ZERO(&state->write_fds);
    FD_ZERO(&state->read_fds);
    poller_state_ = state;
}

Poller::~Poller() {
    auto *state = static_cast<SelectState *>(poller_state_);
    delete state;
}

std::string Poller::poller_name() const {
    return "select";
}

void Poller::add_event(int fd, uint32_t mask) {
    auto *state = static_cast<SelectState *>(poller_state_);
    max_fd_ = std::max(max_fd_, fd);

    if(mask & WS_READABLE) FD_SET(fd, &state->read_fds);
    if(mask & WS_WRITABLE) FD_SET(fd, &state->write_fds);
}

void Poller::delete_event(int fd, uint32_t mask) {
    auto *state = static_cast<SelectState *>(poller_state_);
    if(mask & WS_READABLE){
        FD_CLR(fd, &state->read_fds);
    }
    if(mask & WS_WRITABLE){
        FD_CLR(fd, &state->write_fds);
    }

    if(max_fd_ == fd) {
        int i;
        for (i = max_fd_; i >= 0; i--) {
            if (FD_ISSET(i, &state->read_fds)) {
                break;
            }
            if (FD_ISSET(i, &state->write_fds)) {
                break;
            }
        }
        max_fd_ = i;
    }
}

const std::vector<FileEvent> & Poller::poll(timeval *tv) {
    auto *state = static_cast<SelectState *>(poller_state_);

    fd_set read_fds_copy, write_fds_copy;
    memcpy(&read_fds_copy, &state->read_fds, sizeof(fd_set));
    memcpy(&write_fds_copy, &state->write_fds, sizeof(fd_set));

    int n = select(max_fd_+1, &read_fds_copy, &write_fds_copy, nullptr, tv);
    fired_events_.clear();

    if(n > 0) {
        for (int i = 0; i <= max_fd_; i++) {
            uint32_t mask = 0;
            if (FD_ISSET(i, &read_fds_copy)) {
                mask |= WS_READABLE;
            }
            if (FD_ISSET(i, &write_fds_copy)) {
                mask |= WS_WRITABLE;
            }
            if(mask){
                fired_events_.push_back(FileEvent{i, mask});
            }
        }
    }
    return fired_events_;
}


} // end namespace net
} // namespace ws


#endif