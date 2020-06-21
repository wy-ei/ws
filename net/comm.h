//
// Created by dodo on 2020/6/13.
//

#ifndef _WS_COMM_H_
#define _WS_COMM_H_


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

#define WS_DEBUG 1
using socket_t = int;

class Server;
class Request;
class Response;
class Channel;
class Socket;
class EventLoop;


using Handler = std::function<void(Request &, Response &)>;
using Headers = std::multimap<std::string, std::string>;
using Params = std::unordered_map<std::string, std::string>;

const int KEEPALIVE_TIMEOUT_SECOND = 5;
const size_t REQUEST_BUF_LEN = 4096;

template <int name>
struct EventName{};

#define SP std::shared_ptr


#endif //_WS_COMM_H_
