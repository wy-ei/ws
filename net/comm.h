//
// Created by wy on 2020/6/13.
//

#ifndef WS_COMM_H
#define WS_COMM_H


#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>

#include <sys/select.h>
#include <unistd.h>
#include <cstring>

#include <functional>
#include <vector>
#include <list>
#include <string>
#include <map>
#include <unordered_map>
#include <cassert>
#include <memory>

namespace ws{
namespace net{


#define WS_DEBUG 1
using socket_t = int;

class Channel;
class Socket;
class EventLoop;


} // end namespace net
} // namespace ws

#endif //WS_COMM_H
