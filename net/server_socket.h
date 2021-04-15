#ifndef WS_NET_SERVER_SOCKET_H_
#define WS_NET_SERVER_SOCKET_H_

#include <string>
#include <uni

class ServerSocket {
public:
    ServerSocket();
    virtual int listen(const std::string& address_string, uint16_t port, int backlog);
    virtual int accept();
                    
private:

};


#endif