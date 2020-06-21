//
// Created by dodo on 2020/6/13.
//

#ifndef _WS_REQUEST_H_
#define _WS_REQUEST_H_

#include <map>
#include <utility>
#include <vector>
#include <functional>
#include <ostream>
#include <unordered_map>

#include "../net/comm.h"
#include "../net/Conn.h"
#include "../base/Buffer.h"

class Request {
    friend std::ostream& operator<<(std::ostream& os, const Request& req){
        os << "\n\n>>>>>>>>>>  request >>>>>>>>>>\n";
        os << "bad: <" << req.bad() << ">\n";
        os << req.method << ' ' << req.path << ' ';
        // os << "query: <" << req.query << ">\n";
        os << req.version << "\n";
        os << "heads:{\n";
        for(auto& kv: req.headers){
            os << "    <" << kv.first << ">:<" << kv.second << ">" << '\n';
        }
        os << "}\n";
        os << "body:(" << req.body.size() << ")\n";
        os << req.body << "<eof>\n";
        os << "<<<<<<<<<<<< request <<<<<<<<<<<<\n\n";

        return os;
    }
    using HeadCompleteCallback = std::function<void()>;
public:
    explicit Request(SP<Conn>& conn):conn_(conn){}

    std::string method;
    std::string path;
    Headers headers;
    std::string body;
    std::string version;
    std::string target;
    Params params;  // TODO  params["user"]
    // TODO: query;


    bool has_header(const std::string& key) const;
    void set_header(const std::string& key, const std::string& val);
    std::string get_header_value(const std::string& key) const;

    bool bad() const { return bad_; }

    void execute_parse(const char *data, size_t len);

    bool finished() const { return current_state_ == PARSE_STATE::END; }
    bool keep_alive();

    void set_head_complete_callback(HeadCompleteCallback callback){
        head_complete_callback_ = std::move(callback);
    }

    int reset();
private:
    bool parse_request_line(const char* line, size_t size);
    void parse_header_line(const char* line, size_t size);
    void parse_query_string(const std::string& s);

    Buffer buffer_;

    bool bad_ = false;
    SP<Conn> conn_;


    enum class PARSE_STATE {
        REQUEST_LINE, HEADER, BODY, END
    };
    PARSE_STATE current_state_ = PARSE_STATE::REQUEST_LINE;


    HeadCompleteCallback head_complete_callback_;

    // bool parse_chunked_content();

};


#endif //_WS_REQUEST_H_