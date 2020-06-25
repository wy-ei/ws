//
// Created by wangyu on 2020/6/13.
//
#include <cassert>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <cstdio>
#include <sstream>
#include <iostream>
#include "Response.h"
#include "imp.h"

namespace ws{
namespace http{
using namespace net;

bool Response::has_header(const char *key) const {
    return imp::has_header(headers, key);
}

std::string Response::get_header_value(const char *key, size_t id) const {
    return imp::get_header_value(headers, key, id);
}

size_t Response::get_header_value_count(const char *key) const {
    return imp::get_header_value_count(headers, key);
}

void Response::set_header(const std::string& key, std::string val) {
    if(key == "Transfer-Encoding" && val == "chunked"){
        chunk_encoding_ = true;
    }
    headers.emplace(key, std::move(val));
}

void Response::flush_head() {
    assert(!head_flushed_);
    add_default_headers();
    std::ostringstream os;

    os << version << ' ' << status_ << ' ' << imp::status_message(status_) << "\r\n";

    for(const auto &x: headers){
        os << x.first << ": " << x.second << "\r\n";
    }
    os << "\r\n";
    std::string head = os.str();
    head_flushed_ = true;
    conn_->write(head.data(), head.size());
}

ssize_t Response::write(const char *content, size_t size) {
    if(!head_flushed_) {
        flush_head();
    }

    if(chunk_encoding_){
        write_chunk(content, size);
    }else{
        conn_->write(content, size);
    }
    return size;
}

ssize_t Response::write(const std::string &content) {
    return write(content.data(), content.size());
}


void Response::end() {
    assert(!end_has_called_);
    end_has_called_ = true;

    if(chunk_encoding_){
        write_chunk("", 0);
    }

    if(finish_callback_){
        //finish_callback_();
    }
}

size_t Response::write_chunk(const char *chunk, size_t size) {
    const size_t CHUNK_SIZE_LIMIT = 8100;
    if(size > CHUNK_SIZE_LIMIT){
        write_chunk(chunk, CHUNK_SIZE_LIMIT);
        write_chunk(chunk + CHUNK_SIZE_LIMIT, size - CHUNK_SIZE_LIMIT);
        return size;
    }

    char chunk_buffer[CHUNK_SIZE_LIMIT + 20];

    char *p = chunk_buffer;
    int n = sprintf(p, "%x", static_cast<unsigned int>(size));
    assert(n > 0);

    p = p + n;
    *p = '\r'; p++;
    *p = '\n'; p++;

    if(size > 0){
        memcpy(p, chunk, size);
    }
    p += size;

    *p = '\r'; p++;
    *p = '\n'; p++;
    *p = '\0';

    size = p - chunk_buffer;
    conn_->write(chunk_buffer, size);
    return size;
}


void Response::add_default_headers() {
    if(!has_header("Content-Length")){
        set_header("Transfer-Encoding", "chunked");
    }

    if(!has_header("Content-Type")){
        set_header("Content-Type", "text/plain");
    }
}

bool Response::keep_alive() {
    return get_header_value("Connection") == "Keep-Alive";
}

void Response::reset() {
    LOG_DEBUG << "reset response";
    status_ = -1;
    headers.clear();
    body.clear();
    head_flushed_ = false;
    end_has_called_ = false;
    chunk_encoding_ = false;
}

} // end namespace http
} // namespace ws

