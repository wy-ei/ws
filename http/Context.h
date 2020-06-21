//
// Created by wangyu on 2020/6/18.
//

#ifndef EX1_CONTEXT_H
#define EX1_CONTEXT_H


#include <memory>
#include <utility>

#include "../net/comm.h"
#include "../base/Buffer.h"
#include "../net/Conn.h"
#include "Request.h"
#include "Response.h"
#include "Router.h"
#include "Middleware.h"
#include "../log/logging.h"

class Context: public std::enable_shared_from_this<Context>{
    using RequestCallback = std::function<void(SP<Request> req, SP<Response> res)>;
public:
    explicit Context(SP<Conn>& conn, Router& router, std::vector<SP<Middleware>>& middleware)
        :conn_(conn), router_(router), middleware_(middleware),
        req_(conn), res_(conn){

        set_callbacks();

    }
    ~Context(){
        debug("context die --->\n");
    }

    void on_message(const char* data, size_t len);
    void reset_context();

    void set_callbacks();
    void handle_request();

private:
    void set_default_response_headers();

    RequestCallback request_callback_;

    Router& router_;
    std::vector<std::shared_ptr<Middleware>>& middleware_;

    SP<Conn> conn_;

    Request req_;
    Response res_;
};


#endif //EX1_CONTEXT_H
