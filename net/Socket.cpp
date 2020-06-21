//
// Created by dodo on 2020/6/16.
//

#include "Socket.h"
#include "../log/logging.h"
#include <thread>
#include <stdexcept>


SocketAddress::SocketAddress(const sockaddr* sa) {
    char str[128];
    switch (sa->sa_family){
        case AF_INET:{
            const auto *sin = reinterpret_cast<const sockaddr_in*>(sa);
            if(inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == nullptr){
                debug("error\n");
                return;
            }
            host = str;
            port = ntohs(sin->sin_port);
            break;
        }
        case AF_INET6:{
            const auto *sin6 = reinterpret_cast<const sockaddr_in6*>(sa);
            if(inet_ntop(AF_INET6, &sin6->sin6_addr, str, sizeof(str)) == nullptr){
                return;
            }
            host = str;
            port = ntohs(sin6->sin6_port);
            break;
        }
        case AF_UNIX:{
            const auto *sun = reinterpret_cast<const sockaddr_un*>(sa);
            if(sun->sun_path[0] == '\0'){
                host = "unknown";
            }else{
                snprintf(str, sizeof(str), "%s", sun->sun_path);
                host = str;
            }
            break;
        }
    }
}

std::string SocketAddress::to_string() const {
    return host + ":" + std::to_string(port);
}

Socket::~Socket() {
    //::close(sock_);
}


bool Socket::listen() {
    state_ = State::k_listening;
    debug("listening: %s\n", this->address().to_string().c_str());
    return ::listen(sock_, 5) == 0;
}

bool Socket::bind(const SocketAddress& address) {
    if(has_bound){
        throw std::logic_error("can't bind a socket twice");
    }
    addrinfo hints{};
    addrinfo *result;

    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = family_;
    hints.ai_socktype = type_;
    // hints.ai_flags = flags; // TODO
    hints.ai_protocol = proto_;

    auto service = std::to_string(address.port);
    if(getaddrinfo(address.host.c_str(), service.c_str(), &hints, &result) != 0){
        return false;
    }

    for(addrinfo *p = result; p; p = p->ai_next){
        int true_ = 1;
        setsockopt(sock_, SOL_SOCKET, SO_REUSEPORT, static_cast<void*>(&true_), sizeof(true_));

        if(::bind(sock_, p->ai_addr, p->ai_addrlen) == 0){
            freeaddrinfo(result);
            has_bound = true;
            return true;
        }
    }
    freeaddrinfo(result);
    return false;
}

bool Socket::connect(const SocketAddress& address) {
    addrinfo *result;

    auto service = std::to_string(address.port);
    if(getaddrinfo(address.host.c_str(), service.c_str(), nullptr, &result) != 0){
        return false;
    }

    for(addrinfo *p = result; p; p = p->ai_next){
        if(::connect(sock_, p->ai_addr, p->ai_addrlen) == 0){
            freeaddrinfo(result);
            debug("connected success: %s\n", address.to_string().c_str());
            state_ = State::k_connected;
            return true;
        }
    }
    freeaddrinfo(result);
    return false;
}

Socket Socket::accept() {
    sockaddr_in address{};
    socklen_t len = sizeof(sockaddr_in);
    int conn_fd;

    while (true){
        conn_fd = ::accept(sock_, reinterpret_cast<sockaddr*>(&address), &len);
        if(conn_fd < 0){
            if(errno == EINTR || errno == EAGAIN){
                continue;
            }
            if(errno == EMFILE){
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                continue;
            }
            conn_fd = -1;
            break;
        } else {
            break;
        }
    }
    if(conn_fd < 0){
        // TODO
        debug("accept fail\n");
    }
    Socket sock(this->family_, this->type_, 0, conn_fd);
    char client_address_buff[50];
    inet_ntop(AF_INET, &address.sin_addr, client_address_buff, 50);
    debug("receive tcp connection from %s:%d  - fd: %d\n", client_address_buff, address.sin_port, conn_fd);
    return sock;
}

SocketAddress Socket::address() const{
    sockaddr_storage address{};
    socklen_t len = sizeof(address);
    getsockname(sock_, reinterpret_cast<sockaddr*>(&address), &len);
    return SocketAddress(reinterpret_cast<sockaddr*>(&address));
}


ssize_t Socket::recv(char *buffer, size_t size) {
    while(true){
        ssize_t n = ::recv(sock_, buffer, size, 0);
        if(n < 0){
            if(errno == EINTR){
                continue;
            }
            return -1;
        }
        return n;
    }
}

ssize_t Socket::send(const char *data, size_t size) {
    while(true){
        ssize_t n = ::send(sock_, data, size, 0);
        if(n < 0){
            if(errno == EINTR){
                continue;
            }
            return -1;
        }
        return n;
    }
}

ssize_t Socket::send(const std::string &data) {
    return send(data.data(), data.size());
}

void Socket::shutdown(int how) {
    ::shutdown(sock_, how);
}