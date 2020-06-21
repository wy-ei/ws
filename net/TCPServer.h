//
// Created by dodo on 2020/6/17.
//

#ifndef WS_TCPSERVER_H
#define WS_TCPSERVER_H

#include "comm.h"

#include <atomic>
#include "Socket.h"
#include "EventLoop.h"
#include "Conn.h"

class TCPServer {
    using ConnectionCallback = typename Conn::ConnectCallback;
public:
    static const EventName<1> EVENT_CONNECTION;
    static const EventName<5> EVENT_ERROR;

    TCPServer() = default;
    bool listen(const std::string& host, unsigned short port);

    void set_connection_close_callback(const ConnectionCallback& callback){
        connection_close_callback_ = callback;
    }
    void set_connection_open_callback(const ConnectionCallback& callback){
        connection_callback_ = callback;
    }
private:
    void listen_internal();
    void handle_new_connection(const Socket& client_sock);
    void handle_accept();
    EventLoop* next_io_loop();
    void remove_connection(const SP<Conn>&);

    std::list<EventLoop*> event_loops_;
    EventLoop* main_loop_ { nullptr };

    Socket server_socket_ { AF_INET, SOCK_STREAM, 0 };

    size_t num_thread_ = 20;
    std::atomic<bool> running_ {false};

    std::unordered_map<int, SP<Conn>> connections_;
    ConnectionCallback connection_callback_;
    ConnectionCallback connection_close_callback_;

    std::string host_;
    unsigned short port_ { 0 };
    std::string name_;
};

#endif //WS_TCPSERVER_H
