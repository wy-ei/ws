//
// Created by wangyu on 2020/6/26.
//

#include "../../net/Socket.h"
#include <unistd.h>
#include <sys/wait.h>

using namespace ws::net;

void handle_connection(Socket& sock){
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

void do_accept(Socket& server_socket){
    while (true){
        Socket client = server_socket.accept();
        handle_connection(client);
    }
}

int main(){
    Socket server_socket(AF_INET, SOCK_STREAM);
    server_socket.bind({"0.0.0.0", 8001});
    server_socket.listen();

    for(int i=0;i<5;i++){
        pid_t pid = fork();
        if(pid == 0){
            do_accept(server_socket);
            exit(0);
        }
    }

    while(wait(nullptr) > 0){

    }
}