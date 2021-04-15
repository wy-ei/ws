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
#include "base/noncopyable.h"

#include <cstdint>

namespace ws{
namespace net{


const int INVALID_SOCKET = -1;

struct EndPoint{
    SocketAddress() = default;
    SocketAddress(std::string  _host, unsigned short _port): host(std::move(_host)),port(_port){};
    explicit SocketAddress(const sockaddr *sa);
    std::string host_;
    uint16_t port_;
};

std::ostream& operator<<(std::ostream&, const SocketAddress&);

class Socket {
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

    ~Socket();

    int fd() const { return sock_; }

    SocketAddress address() const;
    SocketAddress peer_address() const;

    ssize_t recv(char *data, size_t size);
    ssize_t send(const char* data, size_t size);
    ssize_t send(const std::string& data);

    void shutdown(int how);
private:
    int family_ { AF_STREAM };
    int type_ {  };
    int proto_ { 0 };
    int sock_ { -1 };
};


class ServerSocket: public Socket {
    enum class State{
        kNotBound, kListening
    };
public:
    ServerSocket() = delete;
    explicit Socket(int fd): sock_(fd){}
    Socket(int family, int type, int proto=0, int fd=-1): family_(family), type_(type), proto_(proto){
        if(fd == -1){
            sock_ = ::socket(family, type, proto);
        }else{
            sock_ = fd;
        }
    }

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

    bool connected() const {
        return state_ == State::k_connected || state_ == State::k_listening;
    }
private:
    State state_ { State::k_disconnected };
    bool has_bound { false };
};

class ClientSocket: public Socket {
private:
    void connect(const EndPoint&);
};


} // end namespace net
} // namespace ws

#endif //WS_SOCKET_H
