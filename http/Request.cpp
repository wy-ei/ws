//
// Created by wangyu on 2020/6/13.
//

#include <cstdlib>
#include <regex>


#include "../log/logging.h"
#include "../utils/str.h"
#include "imp.h"
#include "Request.h"

namespace ws{
namespace http{
using namespace net;

bool Request::keep_alive(){
    if(get_header_value("Context") == "close"){
        return false;
    }
    if(version == "HTTP/1.0" && get_header_value("Context") != "Keep-Alive"){
        return false;
    }
    return true;
}


void Request::parse_query_string(const std::string &s) {
    ws::str::split(s.data(), s.data()+s.size(), '&', [this](const char *b, const char *e){
        std::string key;
        std::string val;
        auto key_end = std::find(b, e, '=');
        key.assign(b, key_end);
        key_end ++;
        if(key_end != e){
            val.assign(key_end, e);
        }
        // this->params.emplace(decode_url(key), decode_url(val));
        this->params.emplace(key, val);
    });
}

bool Request::has_header(const std::string& key) const {
    return imp::has_header(headers, key);
}

void Request::set_header(const std::string& key, const std::string &val) {
    this->headers.emplace(key, val);
}

std::string Request::get_header_value(const std::string &key) const{
    return imp::get_header_value(headers, key);
}


void Request::execute_parse(const char *data, size_t len) {
    buffer_.append(data, len);
    assert(current_state_ != PARSE_STATE::END);

    const char* CRLF = "\r\n";
    while(current_state_ != PARSE_STATE::END){
        if(current_state_ == PARSE_STATE::REQUEST_LINE){
            const char* crlf = buffer_.find(CRLF, 2);
            if(!crlf){
                return;
            }
            len = crlf - buffer_.peek() + 2;
            bool success = parse_request_line(buffer_.peek(), len);
            buffer_.consume(len);
            if(success){
                current_state_ = PARSE_STATE::HEADER;
            }else{
                bad_ = true;
                current_state_ = PARSE_STATE::END;
                return;
            }
        }
        else if(current_state_ == PARSE_STATE::HEADER){
            const char* crlf = buffer_.find(CRLF,2);
            if(!crlf){
                return;
            }
            len = crlf - buffer_.peek() + 2;
            parse_header_line(buffer_.peek(), len);

            if(strncmp(buffer_.peek(), CRLF, 2) == 0){
                if(this->method == "POST" || this->method == "PUT"){
                    current_state_ = PARSE_STATE::BODY;
                }else{
                    current_state_ = PARSE_STATE::END;
                }
            }
            buffer_.consume(len);
        }
        else if(current_state_ == PARSE_STATE::BODY){
            // TODO
            current_state_ = PARSE_STATE::END;
            return;
        }
    }
    if(parse_complete_callback_){
        parse_complete_callback_();
    }
}


bool Request::parse_request_line(const char* p, size_t size) {
    const static std::regex re(
        R"((GET|HEAD|POST|PUT|DELETE|CONNECT|OPTIONS|TRACE|PATCH|PRI) (([^?]+?)(?:\?(.*?))?) (HTTP/1\.[01])\r\n)"
    );

    std::string line(p, size);

    std::cmatch m;
    if(std::regex_match(line.c_str(), m, re)){
        this->version = std::string(m[5]);
        this->method = std::string(m[1]);
        this->target = std::string(m[2]);
        this->path = std::string(m[3]);

        auto len = std::distance(m[4].first, m[4].second);
        if(len > 0){
            parse_query_string(m[4].str());
        }
        return true;
    }

    return false;
}

void Request::parse_header_line(const char *line, size_t size) {
    const char* end = line + size - 2;
    auto p = std::find(line, end, ':');
    if(p == end){
        return;
    }
    auto key_end = p;
    p++; // skip ':'
    while(p < end && isspace(*p)){
        p++;
    }
    if(p < end){
        this->headers.emplace(std::string(line, key_end), std::string(p, end));
    }
}

int Request::reset() {
    LOG_DEBUG << "reset request";
    method.clear();
    path.clear();
    headers.clear();
    body.clear();
    version.clear();
    target.clear();
    params.clear();
    current_state_ = PARSE_STATE::REQUEST_LINE;
}


} // end namespace http
} // namespace ws