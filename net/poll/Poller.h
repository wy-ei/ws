//
// Created by 王钰 on 2020/8/6.
//

#ifndef WS_POLLER_H
#define WS_POLLER_H

#include <vector>
#include <unordered_map>
#include <functional>
#include <queue>
#include "net/comm.h"



#ifdef __linux__
#define HAVE_EPOLL 1
#endif


namespace ws{
namespace net{

class FileEvent{
public:
    int fd;
    uint32_t mask;
};

class Poller {
public:
    Poller();
    ~Poller();
    void add_event(int fd, uint32_t mask);
    void delete_event(int fd, uint32_t mask);
    const std::vector<FileEvent>& poll(timeval* tv);
    std::string poller_name() const;
protected:
    void *poller_state_;
    int max_fd_;
    std::vector<FileEvent> fired_events_;
};



}
}




#endif //WS_POLLER_H
