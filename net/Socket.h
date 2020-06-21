//
// Created by wangyu on 2020/6/16.
//

#ifndef WS_SOCKET_H
#define WS_SOCKET_H

#include "comm.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/un.h>

#include <utility>


const int INVALID_SOCKET = -1;

struct SocketAddress{
    SocketAddress() = default;
    SocketAddress(std::string  _host, unsigned short _port): host(std::move(_host)),port(_port){};
    explicit SocketAddress(const sockaddr *sa);
    std::string to_string() const;


    std::string host;
    unsigned short port {0};
};

class Socket{
    enum class State{
        k_connected, k_disconnected, k_listening
    };
public:
    explicit Socket(int fd): sock_(fd){}
    Socket(int family, int type, int proto=0, int fd=-1): family_(family), type_(type), proto_(proto){
        if(fd == -1){
            sock_ = ::socket(family, type, proto);
        }else{
            sock_ = fd;
        }
    }
    ~Socket();

    Socket accept();
    bool connect(const SocketAddress& address);
    bool bind(const SocketAddress& address);
    bool listen();

    int fd() const { return sock_; }

    bool connected() const { return state_ == State::k_connected; }

    SocketAddress address() const;

    ssize_t recv(char *data, size_t size);
    ssize_t send(const char* data, size_t size);
    ssize_t send(const std::string& data);

    void shutdown(int how);
private:
    State state_ { State::k_disconnected };
    bool has_bound { false };
    int family_{};
    int type_{};
    int proto_{ 0 };
    int sock_;
};


#endif //WS_SOCKET_H
