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

        list_.emplace_back(node);
        auto it = list_.end();
        map_[id] = --it;
    }

    void remove(const std::shared_ptr<Channel>& channel){
        int id = channel->fd();
        auto it = map_.find(id);
        if(it != map_.end()){
            list_.erase(it->second);
            map_.erase(id);
        }
    }

    void handle_timeout_channels(const std::function<void(std::shared_ptr<Channel>& channel)>& func){
        long now = current_time_ms();

        auto it = list_.begin();
        while(it != list_.end() && it->time_ < now){
            map_.erase(it->channel_->fd());
            func(it->channel_);
            it++;
        }
        list_.erase(list_.begin(), it);
    }

private:
    static long current_time_ms(){
        long now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        return now;
    }


    long timeout_ms_;

    using list_iterator = typename std::list<TimeNode>::iterator;
    using map_iterator = typename std::unordered_map<key_t, list_iterator>::iterator;

    std::list<TimeNode> list_;
    std::unordered_map<int, list_iterator> map_;
};

} // end namespace net
} // namespace ws

#endif //WS_TIMEOUT_H
