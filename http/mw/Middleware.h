//
// Created by wangyu on 2020/6/14.
//

#ifndef WS_MIDDLEWARE_H
#define WS_MIDDLEWARE_H

#include "http/comm.h"

namespace ws{
namespace http{

class Middleware{
public:
    virtual ~Middleware() = default;
    virtual void call(Request &req, Response &res) = 0;
};


} // end namespace http
} // end namespace ws

#endif //WS_MIDDLEWARE_H
