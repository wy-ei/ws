//
// Created by wangyu on 2020/6/17.
//

#ifndef WS_TCP_SERVER_H
#define WS_TCP_SERVER_H


#include <atomic>
#include <string>
#include <list>

#include "net/server_socket.h"

namespace ws{
namespace net{

class EventLoop;
class ServerSocket;

class TCPServer {
public:
    TCPServer() = default;
    explicit TCPServer(int io_thread_num): io_thread_num_(io_thread_num){}
    bool listen(const std::string& host, unsigned short port);

    void set_connection_open_callback(const ConnectionCallback& callback){
        connection_callback_ = callback;
    }
private:
    void listen_internal();
    void handle_new_connection(ClientSocket client_sock);
    void handle_accept();
    EventLoop* next_io_loop();

    std::list<EventLoop*> event_loops_;
    EventLoop* main_loop_ { nullptr };

    ServerSocket server_socket_ { AF_INET, SOCK_STREAM, 0 };

    size_t io_thread_num_ { 1 };
    std::atomic<bool> running_ {false};

    ConnectionCallback connection_callback_;

    std::string host_;
    unsigned short port_ { 0 };
};


} // namespace net
} // namespace ws

#endif //WS_TCP_SERVER_H
