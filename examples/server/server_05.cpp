//
// Created by wangyu on 2020/6/26.
//

#include <unistd.h>
#include <sys/wait.h>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

#include "../../net/Socket.h"
#include "../../base/ThreadPool.h"

using namespace ws::net;
using namespace ws::base;

std::queue<Socket> client_queue;
std::mutex mutex_;
std::condition_variable cond_;

void handle_connection(){
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [&]{
        return !client_queue.empty();
    });
    Socket client = client_queue.front();
    client_queue.pop();
    lock.unlock();

    char buffer[4096];
    while(true){
        ssize_t n = client.recv(buffer, sizeof(buffer));
        if(n > 0){
            client.send(buffer, n);
        }else{
            client.shutdown(SHUT_RDWR);
            break;
        }
    }
}


int main(){
    Socket server_socket(AF_INET, SOCK_STREAM);
    server_socket.bind({"0.0.0.0", 8001});
    server_socket.listen();

    ThreadPool pool(10);

    for(int i=0;i<10;i++){
        pool.enqueue(handle_connection);
    }

    while (true) {
        Socket sock = server_socket.accept();
        std::unique_lock<std::mutex> lock(mutex_);
        client_queue.push(std::move(sock));
        cond_.notify_one();
    }
}