//
// Created by dodo on 2020/6/17.
//

#include "TCPServer.h"

#include "Socket.h"
#include "../base/ThreadPool.h"

using std::shared_ptr;

bool TCPServer::listen(const std::string& host, unsigned short port) {
    bool success = server_socket_.bind({host, port});
    assert(success);

    size_t num_thread = 1;
    std::unique_ptr<ThreadPool> thread_pool(new ThreadPool(num_thread));

    for(size_t i=0;i<num_thread;i++){
        thread_pool->enqueue([this](){
            auto loop = new EventLoop();
            event_loops_.push_back(loop);
            loop->loop();
        });
    }

    main_loop_ = new EventLoop();
    main_loop_->run_in_loop([this](){
        listen_internal();
    });

    main_loop_->loop();
    return true;
}

void TCPServer::listen_internal() {
    server_socket_.listen();
    auto accept_channel_ = std::make_shared<Channel>(main_loop_, server_socket_.fd());
    accept_channel_->set_read_callback(std::bind(&TCPServer::handle_accept, this));
    accept_channel_->enable_reading();
}

void TCPServer::handle_accept() {
    debug("accept\n");
    main_loop_->assert_in_loop_thread();
    Socket client = server_socket_.accept();
    if(client.fd() > 0){
        handle_new_connection(client);
    }else{
        debug("accept fail\n");
    }
}

void TCPServer::handle_new_connection(const Socket& client_sock) {
    auto loop = next_io_loop();
    auto conn = std::make_shared<Conn>(loop, client_sock, server_socket_.address());

    connections_[conn->fd()] = conn;

    conn->set_close_callback(std::bind(&TCPServer::remove_connection, this, conn));

    if(connection_callback_){
        conn->set_connect_callback(connection_callback_);
    }else{
        conn->set_connect_callback([this](const SP<Conn>& conn){
            debug("local address: %s  peer address: %s\n", conn->socket().address().to_string().c_str(),
                    server_socket_.address().to_string().c_str());
        });
        debug("no handle\n");
    }
    loop->run_in_loop(std::bind(&Conn::init, conn));
}

EventLoop* TCPServer::next_io_loop() {
    auto loop = event_loops_.back();
    if(event_loops_.size() > 1){
        // 取最后一个，并把最后一个放到最前面
        event_loops_.pop_back();
        event_loops_.push_front(loop);
    }
    return loop;
}

void TCPServer::remove_connection(const SP<Conn>& conn) {
    debug("close tcpserver->\n");
    main_loop_->run_in_loop([this, conn]{
        connections_.erase(conn->fd());
        EventLoop* io_loop = conn->get_loop();
        io_loop->enqueue(std::bind(&Conn::destroy, conn));
    });
}


