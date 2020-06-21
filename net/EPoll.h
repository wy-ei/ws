//
// Created by wangyu on 2020/6/16.
//

#ifndef WS_EPOLL_H
#define WS_EPOLL_H

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <fcntl.h>
#include <memory>
#include <vector>

#include "Channel.h"
#include "../log/logging.h"



class EPoll{
    static const int k_events_max_size = 10;
public:
    explicit EPoll()
        :events_(k_events_max_size){

        epoll_fd_ = epoll_create(5);
        assert(epoll_fd_ >= 0);
    }

    ~EPoll(){
        ::close(epoll_fd_);
    }

    std::vector<SP<Channel>> wait(int timeout_ms);
    std::vector<SP<Channel>> get_activate_channels(int n);

    void add_channel(const SP<Channel>& channel);
    void remove_channel(const SP<Channel>&  channel);
    bool has_channel(const SP<Channel>& channel) const;
    int fd(){ return epoll_fd_; }
private:
    void update(int op, const SP<Channel>& channel);

    std::unordered_map<int, SP<Channel>> fd_to_channel_;

    int epoll_fd_;
    std::vector<epoll_event> events_;

};


#endif //WS_EPOLL_H
