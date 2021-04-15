#ifndef WS_NET_SERVER_SOCKET_H_
#define WS_NET_SERVER_SOCKET_H_

#include <string>

class ServerSocket {
public:
    ServerSocket();

    virtual int listen(const std::string& address_string, uint16_t port, int backlog);
    virtual int GetLocalAddress(IPEndPoint* address) const = 0;
    virtual int Accept(std::unique_ptr<StreamSocket>* socket,
                     CompletionOnceCallback callback) = 0;

  // Accepts connection. Callback is called when new connection is accepted.
  // Note: |peer_address| may or may not be populated depending on the
  // implementation.
  virtual int Accept(std::unique_ptr<StreamSocket>* socket,
                     CompletionOnceCallback callback,
                     IPEndPoint* peer_address);
                    
private:
    DISALLOW_COPY_AND_ASSIGN(ServerSocket);

};


#endif