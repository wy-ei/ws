//
// Created by wangyu on 2020/6/13.
//

#ifndef EX1_RESPONSE_H
#define EX1_RESPONSE_H

#include <utility>

#include "./comm.h"
#include "imp.h"
#include "../net/Conn.h"

namespace ws{
namespace http{
using namespace net;

class Response {
    friend std::ostream& operator<<(std::ostream& os, const Response& res){
        os << ">>>>>>>>>>  response >>>>>>>>>>\n";
        os << "response finished: <" << res.finished() << ">\n";
        os << res.version << ' ' << res.status_ << ' ' << imp::status_message(res.status_) << '\n';
        os << "head:{\n";
        for(auto& kv: res.headers){
            os << "    <" << kv.first << ">:<" << kv.second << ">" << '\n';
        }
        os << "}\n";
        //os << "body:(" << res.body.size() << ")\n";
        //os << req.body << "<eof>\n";
        os << "<<<<<<<<<<<< response <<<<<<<<<<<<\n";

        return os;
    }
    using FinishCallback = std::function<void()>;
public:
    explicit Response(std::shared_ptr<Conn>& conn): conn_(conn){}

    bool has_header(const char *key) const;
    void set_header(const std::string& key, std::string val);
    std::string get_header_value(const char *key, size_t id = 0) const;
    size_t get_header_value_count(const char *key) const;

    void set_finished_callback(FinishCallback callback){
        finish_callback_ = std::move(callback);
    }

    ssize_t write(const char* content, size_t size);
    ssize_t write(const std::string& content);

    void end();
    bool finished() const { return end_has_called_; }

    void set_status(int status){
        status_ = status;
    }
    bool keep_alive();
    void reset();
private:
    void flush_head();
    void add_default_headers();
    size_t write_chunk(const char* chunk, size_t size);

    int status_ = -1;
    std::string version = "HTTP/1.1";
    Headers headers;
    std::string body;

    std::shared_ptr<Conn> conn_;


    FinishCallback finish_callback_;
    bool chunk_encoding_ = false;
    bool head_flushed_ = false;
    bool end_has_called_ = false;
};

} // end namespace http
} // namespace ws

#endif //EX1_RESPONSE_H
