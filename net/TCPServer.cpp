//
// Created by wangyu on 2020/6/17.
//

#include "TCPServer.h"

#include "Socket.h"
#include "../base/ThreadPool.h"
#include "../log/logging.h"


namespace ws{
namespace net{


using std::shared_ptr;


bool TCPServer::listen(const std::string& host, unsigned short port) {
    bool success = server_socket_.bind({host, port});
    assert(success);


    std::unique_ptr<ThreadPool> thread_pool(new ThreadPool(io_thread_num_));
    for(size_t i=0;i<io_thread_num_;i++){
        thread_pool->enqueue([this](){
            auto loop = new EventLoop();
            event_loops_.push_back(loop);
            loop->start();
        });
    }

    main_loop_ = new EventLoop();
    main_loop_->run_in_loop([this](){
        listen_internal();
    });
    main_loop_->start();
    return true;
}

void TCPServer::listen_internal() {
    main_loop_->assert_in_loop_thread();
    server_socket_.listen();
    auto accept_channel_ = std::make_shared<Channel>(server_socket_.fd());
    accept_channel_->set_read_callback(std::bind(&TCPServer::handle_accept, this));
    accept_channel_->enable_reading();
    accept_channel_->name = "accept channel";
    main_loop_->add_channel(accept_channel_);
}

void TCPServer::handle_accept() {
    main_loop_->assert_in_loop_thread();
    Socket client = server_socket_.accept();
    if(client.fd() > 0){
        handle_new_connection(std::move(client));
    }else{
        LOG_ERROR << "accept failed";
    }
}

void TCPServer::handle_new_connection(Socket client_sock) {
    auto loop = next_io_loop();
    auto conn = std::make_shared<Conn>(loop, std::move(client_sock), server_socket_.address());

    if(!connection_callback_){
        LOG_DEBUG << "no connection handle for: " << conn->socket().address();
        return;
    }
    connection_callback_(conn);

    auto channel = conn->build_channel();
    loop->enqueue(std::bind(&EventLoop::add_channel, loop, channel));
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



} // end namespace net
} // namespace ws
