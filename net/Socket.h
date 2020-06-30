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
#include <ostream>
#include <utility>

namespace ws{
namespace net{


const int INVALID_SOCKET = -1;

struct SocketAddress{
    SocketAddress() = default;
    SocketAddress(std::string  _host, unsigned short _port): host(std::move(_host)),port(_port){};
    explicit SocketAddress(const sockaddr *sa);
    std::string to_string() const;
    std::string host;
    unsigned short port {0};
};

std::ostream& operator<<(std::ostream&, const SocketAddress&);

class Socket{
    enum class State{
        k_connected, k_disconnected, k_listening
    };
public:
    Socket() = delete;
    explicit Socket(int fd): sock_(fd){}
    Socket(int family, int type, int proto=0, int fd=-1): family_(family), type_(type), proto_(proto){
        if(fd == -1){
            sock_ = ::socket(family, type, proto);
        }else{
            sock_ = fd;
        }
    }

    Socket(Socket&) = delete;
    Socket(const Socket&) = delete;
    Socket(const Socket&&) = delete;
    Socket(Socket&& sock) noexcept {
        swap(sock);
    }

    void swap(Socket& sock) noexcept{
        std::swap(state_, sock.state_);
        std::swap(has_bound, sock.has_bound);
        std::swap(family_, sock.family_);
        std::swap(type_, sock.type_);
        std::swap(proto_, sock.proto_);
        std::swap(sock_, sock.sock_);
    }

    Socket& operator=(const Socket&) = delete;

    Socket& operator=(Socket&& sock) noexcept{
        if (connected())
            shutdown(SHUT_RDWR);
        swap(sock);
        return *this;
    }

    ~Socket();

    Socket accept();
    bool connect(const SocketAddress& address);
    bool bind(const SocketAddress& address);
    bool listen();

    int fd() const { return sock_; }

    bool connected() const {
        return state_ == State::k_connected || state_ == State::k_listening;
    }

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
    int sock_ { -1 };
};

} // end namespace net
} // namespace ws

#endif //WS_SOCKET_H
