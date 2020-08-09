//
// Created by wangyu on 2020/6/17.
//

#ifndef WS_TIMEOUT_H
#define WS_TIMEOUT_H

#include <memory>
#include <queue>
#include <vector>
#include <utility>
#include <chrono>
#include <list>
#include <unordered_map>
#include <functional>
#include <memory>


#include "comm.h"
#include "Channel.h"

namespace ws{
namespace net{

struct TimeNode{
    long time_;
    std::shared_ptr<Channel> channel_;

    TimeNode(std::shared_ptr<Channel> channel, long time):channel_(std::move(channel)), time_(time){}
};


class Timeout {
public:
    explicit Timeout(long timeout_ms): timeout_ms_(timeout_ms){}

    void add(const std::shared_ptr<Channel>& channel){
        long time_ms = current_time_ms() + timeout_ms_;

        int id = channel->fd();
        TimeNode node(channel, time_ms);

        this->remove(channel);

        list_.push_back(node);
        auto it = list_.end();
        map_[id] = --it;
    }

    void remove(const std::shared_ptr<Channel>& channel){
        int id = channel->fd();
        auto it = map_.find(id);
        if(it != map_.end()){
            list_.erase(it->second);
            map_.erase(it);
        }
    }

    std::vector<std::shared_ptr<Channel>> get_timeout_channels(){
        long now = current_time_ms();

        std::vector<std::shared_ptr<Channel>> channels;
        auto it = list_.begin();
        while(it != list_.end() && it->time_ < now){
            channels.push_back(it->channel_);
            it++;
        }
        return channels;
    }

private:
    static long current_time_ms(){
        using namespace std::chrono;
        auto duration_ns = steady_clock::now().time_since_epoch();
        auto duration_ms = duration_cast<milliseconds>(duration_ns);
        return duration_ms.count();
    }

    long timeout_ms_ { 5000 };

    using list_iterator = typename std::list<TimeNode>::iterator;
    using map_iterator = typename std::unordered_map<key_t, list_iterator>::iterator;

    std::list<TimeNode> list_;
    std::unordered_map<int, list_iterator> map_;
};

} // end namespace net
} // namespace ws

#endif //WS_TIMEOUT_H
