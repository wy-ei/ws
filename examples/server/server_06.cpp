//
// Created by wangyu on 2020/6/26.
//

#include <unistd.h>
#include <sys/epoll.h>

#include "../../net/Socket.h"

using namespace ws::net;


int main(){
    Socket server_socket(AF_INET, SOCK_STREAM);
    server_socket.bind({"0.0.0.0", 8001});
    server_socket.listen();

    int efd = epoll_create(10);
    epoll_event ev{};
    bzero(&ev, sizeof(ev));
    ev.data.fd = server_socket.fd();
    ev.events = EPOLLIN | EPOLLHUP;
    epoll_ctl(efd, EPOLL_CTL_ADD, server_socket.fd(), &ev);

    std::unordered_map<int, Socket> fd_to_socket;

    epoll_event events[100];
    for(;;){
        int n = epoll_wait(efd, events, 100, -1);

        for(int i=0;i<n;i++){
            int fd = events[i].data.fd;
            if(fd == server_socket.fd()){
                Socket client = server_socket.accept();
                fd_to_socket.emplace(client.fd(), client);
                ev.data.fd = client.fd();
                ev.events = EPOLLIN;
                epoll_ctl(efd, EPOLL_CTL_ADD, client.fd(), &ev);
            }else{
                Socket client = fd_to_socket[fd];
                char buffer[1024];
                int n = client.recv(buffer, sizeof(buffer));
                if(n <= 0){
                    epoll_ctl(efd, EPOLL_CTL_DEL,  fd, nullptr);
                    fd_to_socket.erase(fd);
                }else{
                    client.send(buffer, n);
                }
            }
        }
    }
}