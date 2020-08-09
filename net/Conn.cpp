//
// Created by wangyu on 2020/6/20.
//

#include <thread>
#include <utility>
#include "EventLoop.h"
#include "Conn.h"
#include "../log/logging.h"

namespace ws{
namespace net{
using namespace base;


Conn::Conn(EventLoop* loop, Socket sock, SocketAddress server_address)
    :sock_(std::move(sock)), event_loop_(loop), server_address_(std::move(server_address)){
    LOG_DEBUG << "new conn:" << sock_.fd();

    state_ = State::k_connected;
}

Conn::~Conn() {
    LOG_DEBUG << "conn die:" << sock_.fd();
}


ssize_t Conn::write(const char *buffer, size_t size) {
    if(size <= 0){
        return size;
    }
    write_buffer_.append(buffer, size);
    auto channel = channel_.lock();
    channel->enable_writing();
    event_loop_->update_channel(channel);
    return size;
}

void Conn::set_message_callback(MessageCallback callback) {
    message_callback_ = std::move(callback);
}

void Conn::handle_read() {
    assert(channel_.lock()->is_enable_reading());

    LOG_DEBUG << "Conn::handle read: " << fd();
    char buffer[65536];
    ssize_t n = sock_.recv(buffer, 65536);
    if(n > 0){
        if(message_callback_){
            message_callback_(buffer, n);
        }
    }else if(n == 0){
        close_read();
    }else{
        // error
        close_connection();
    }
}

void Conn::handle_write() {
    LOG_DEBUG << "Conn::handle write: " << fd();
    event_loop_->assert_in_loop_thread();

    auto channel = channel_.lock();
    assert(channel->is_enable_writing());

    int n = sock_.send(write_buffer_.peek(), write_buffer_.readable_size());
    if(n > 0){
        write_buffer_.consume(n);
        if(write_buffer_.readable_size() == 0){
            // 当前没有数据可写了，而且用户不会在写入内容了
            // 但是并不 shutdown 读端，这样可以在对方关闭后，触发 close 回调
            channel->disable_writing();
            event_loop_->update_channel(channel);
        }
    }else{
        close_connection();
    }
}

void Conn::close_read() {
    LOG_DEBUG << "close_read: " << fd();

    assert(state_ == State::k_connected);
    state_ = State::k_read_closed;
    auto channel = channel_.lock();
    channel->disable_reading();
    event_loop_->run_in_loop([this,channel]{
        event_loop_->update_channel(channel);
        if(!channel->is_enable_writing()){
            close_connection();
        }
    });
}

std::shared_ptr<Channel> Conn::build_channel() {
    auto channel = std::make_shared<Channel>(fd());
    channel->set_read_callback(std::bind(&Conn::handle_read, this));
    channel->set_write_callback(std::bind(&Conn::handle_write, this));
    channel->enable_timeout();
    channel->enable_reading();
    channel->conn = shared_from_this();
    channel->name = sock_.peer_address().to_string();
    channel_ = channel;
    return channel;
}

void Conn::close_connection() {
    if(state_ == State::k_disconnected){
        return;
    }
    state_ = State::k_disconnected;
    if(close_callback_){
        close_callback_(shared_from_this());
    }
    auto channel = channel_.lock();
    if(channel){
        channel->close();
        event_loop_->remove_channel(channel);
    }
    sock_.shutdown(SHUT_RDWR);
}

} // end namespace net
} // namespace ws