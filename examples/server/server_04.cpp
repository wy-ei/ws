//
// Created by wangyu on 2020/6/26.
//

#include "../../net/Socket.h"
#include <unistd.h>
#include <sys/wait.h>
#include <thread>

using namespace ws::net;

void handle_connection(Socket sock){
    char buffer[4096];
    while(true){
        ssize_t n = sock.recv(buffer, sizeof(buffer));
        if(n > 0){
            sock.send(buffer, n);
        }else{
            sock.shutdown(SHUT_RDWR);
            break;
        }
    }
}


int main(){
    Socket server_socket(AF_INET, SOCK_STREAM);
    server_socket.bind({"0.0.0.0", 8001});
    server_socket.listen();

    while(true){
        Socket client = server_socket.accept();

        std::thread t([=]{
           handle_connection(client);
        });
        t.detach();
    }
}