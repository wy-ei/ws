//
// Created by wangyu on 2020/6/20.
//

#include "Conn.h"
#include <thread>
#include <utility>
#include "EventLoop.h"
#include "../log/logging.h"

namespace ws{
namespace net{
using namespace base;


Conn::Conn(EventLoop* loop, const Socket& sock, SocketAddress  peer_address)
    :sock_(sock), loop_(loop), peer_address_(std::move(peer_address)),
    channel_(std::make_shared<Channel>(loop, sock.fd())){
    LOG_DEBUG << "new conn:" << sock.fd();
}

Conn::~Conn() {
    LOG_DEBUG << "conn die:" << sock_.fd();
}

ssize_t Conn::read(char *buffer, size_t size) {
    return sock_.recv(buffer, size);
}

ssize_t Conn::write(const char *buffer, size_t size) {
    write_buffer_.append(buffer, size);
    if(!channel_->is_writing()){
        channel_->enable_writing();
    }
    return size;
}

void Conn::handle_read() {
    LOG_DEBUG << "Conn::handle read";
    char buffer[65536];
    ssize_t n = read(buffer, 65536);
    if(n > 0){
        if(message_callback_){
            message_callback_(buffer, n);
        }
    }else if(n == 0){
        handle_close();
    }else{
        handle_error();
    }
}

void Conn::handle_write() {
    LOG_DEBUG << "Conn::handle write";
    loop_->assert_in_loop_thread();
    if(!channel_->is_writing()){
        assert(0); // this shouldn't happen
        return;
    }
    int n = sock_.send(write_buffer_.peek(), write_buffer_.readable_size());
    if(n > 0){
        write_buffer_.consume(n);
        if(write_buffer_.readable_size() == 0){
            channel_->disable_writing();
            // 用户已经主动关闭了连接，此时可以 shutdown 底层的 socket 了
            // 但是并不 shutdown 读端，这样可以在对方关闭后，触发 close 回调
            if(state_ == State::k_disconnected){
                loop_->run_in_loop([this]{
                    sock_.shutdown(SHUT_WR);
                });
            }
        }

    }
}

void Conn::handle_close() {
    LOG_DEBUG << "handle close";

    assert(state_ == State::k_disconnecting || state_ == State::k_connected);

    state_ = State::k_disconnected;
    channel_->disable_all();

    if(close_callback_){
        std::shared_ptr<Conn> self(shared_from_this());
        close_callback_(self);
    }
}

void Conn::handle_error() {
    LOG_DEBUG << "handle error";
}


void Conn::init() {
    loop_->assert_in_loop_thread();
    assert(state_ == State::k_connecting);
    state_ = State::k_connected;
    setup_channel();
    loop_->wakeup();
    connect_callback_(shared_from_this());
}

void Conn::setup_channel() {
    channel_->set_read_callback(std::bind(&Conn::handle_read, this));
    channel_->set_write_callback(std::bind(&Conn::handle_write, this));
    channel_->set_error_callback(std::bind(&Conn::handle_error, this));
    channel_->set_close_callback(std::bind(&Conn::handle_close, this));
    loop_->enqueue(std::bind(&Channel::enable_reading, channel_));
}


void Conn::destroy() {
    LOG_DEBUG << "conn destroy:" << fd();
    loop_->assert_in_loop_thread();
    if(state_ == State::k_connected){
        state_ = State::k_disconnected;
        channel_->disable_all();
    }
    channel_->remove_self_from_loop();
}

// 用户主动关闭：
// 用户写完了数据，然后关闭连接，但是数据还存储在缓冲区里，此时并不能直接关闭，需要等到下次调用 handle_write 写完数据后再关闭
// 此时没有数据需要发送，缓冲区为空，此时可以直接关闭
void Conn::shutdown() {
    // this line don't work
    // state_.compare_exchange_strong(State::k_connected, State::k_disconnected);

    State connected = State::k_connected;
    State disconnecting = State::k_disconnecting;

    if(state_.compare_exchange_strong(connected, disconnecting)){
        loop_->run_in_loop([this]{
           // no more data to write
            if(write_buffer_.readable_size() == 0){
               sock_.shutdown(SHUT_WR);
           }
        });
    }

}

} // end namespace net
} // namespace ws