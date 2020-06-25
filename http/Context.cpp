//
// Created by wangyu on 2020/6/18.
//

#include "Context.h"

#include <iostream>

namespace ws{
namespace http{

void Context::on_message(const char *data, size_t len) {
    std::string message(data, len);
    LOG_DEBUG << "message comming:" << message;
    req_.execute_parse(data, len);
}

void Context::set_callbacks() {
    req_.set_parse_complete_callback(std::bind(&Context::handle_request, this));
    res_.set_finished_callback(std::bind(&Context::reset_context, this));

}

void Context::reset_context() {
    std::cout << res_;
    if(req_.keep_alive() && res_.keep_alive() && !req_.bad()){
        req_.reset();
        res_.reset();
    }else{
        conn_->shutdown();
    }
}

void Context::handle_request() {
    LOG_DEBUG << "handle request";
    set_default_response_headers();

    if(req_.bad()){
        res_.set_status(400);
        res_.end();
        return;
    }

    for(auto& mw: middleware_){
        mw->call(req_, res_);
        if(res_.finished()){
            return;
        }
    }

    router_.handle(req_, res_);
    if(!res_.finished()){
        res_.set_status(404);
        res_.write("not found");
        res_.end();
    }
}

void Context::set_default_response_headers() {
    if (req_.keep_alive()) {
        res_.set_header("Connection", "Keep-Alive");
    }
    res_.set_header("HTTPServer", "WS/0.1");
}

} // end namespace http
} // namespace ws
