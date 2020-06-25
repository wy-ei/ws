//
// Created by wangyu on 2020/6/20.
//

#ifndef WS_CONN_H
#define WS_CONN_H

#include <utility>
#include <atomic>

#include "comm.h"
#include "Socket.h"
#include "../base/Buffer.h"


namespace ws{
namespace net{
using namespace base;

class Conn : public std::enable_shared_from_this<Conn>{
    /*
     * 初始时： k_connecting
     * 当回调函数和 channel 等都设置好了之后，状态变为 k_connected
     *
     * 当用户主动关闭连接后，此时可能还存在一些数据没有发完，正在等到 epoll 给 write 的机会，此时状态变为 k_disconnecting
     * 另外，如果对端主动关闭了，就会调用 close_callback 在这里会设置状态为 k_disconnecting
     *
     * 当 write 被触发后，handle_write 写完了缓冲区里的数据后，
    **/

    enum class State:int{
        k_connecting, k_connected, k_disconnecting, k_disconnected
    };

public:
    using ConnectCallback = std::function<void(std::shared_ptr<Conn>)>;
    using MessageCallback = std::function<void(const char*, size_t)>;
    using HighWaterMarkCallback = std::function<void(size_t)>;
    using TimeoutCallback = std::function<void(size_t)>;
    using CloseCallback = std::function<void(const std::shared_ptr<Conn>&)>;
    using ErrorCallback = std::function<void()>;
public:
    Conn() = delete;
    Conn(EventLoop* loop, const Socket& sock, SocketAddress peer_address);
    ~Conn();

    void init();
    void destroy();
    void shutdown();

    EventLoop* get_loop(){ return loop_; }

    ssize_t read(char *buffer, size_t size);
    ssize_t write(const char* data, size_t size);
    int fd() const { return sock_.fd(); }
    const Socket& socket() const { return sock_; }


    bool connected() const { return state_ == State::k_connected; }
    bool disconnected() const { return state_ == State::k_disconnected; }

    void handle_read();
    void handle_write();
    void handle_close();
    void handle_error();

    void setup_channel();

    void set_connect_callback(ConnectCallback callback){
        connect_callback_ = std::move(callback);
    }
    void set_message_callback(MessageCallback callback){
        message_callback_ = std::move(callback);
    }
    void set_close_callback(CloseCallback callback){
        close_callback_ = std::move(callback);
    }
    void set_timeout_callback(TimeoutCallback callback){
        timeout_callback_ = std::move(callback);
    }
    void set_error_callback(ErrorCallback callback){
        error_callback_ = std::move(callback);
    }

    void* context(){ return context_; }
    void context(void* ctx) { context_ = ctx; }
private:
    ConnectCallback connect_callback_;
    MessageCallback message_callback_;
    HighWaterMarkCallback high_water_mark_callback_;
    TimeoutCallback timeout_callback_;
    CloseCallback close_callback_;
    ErrorCallback error_callback_;

    EventLoop* loop_;
    std::shared_ptr<Channel> channel_;
    std::atomic<State> state_ { State::k_connecting };

    Socket sock_;
    SocketAddress peer_address_;
    void* context_ { nullptr };
    Buffer write_buffer_;
};

} // end namespace net
} // namespace ws

#endif //WS_CONN_H
