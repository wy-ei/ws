//
// Created by wangyu on 2020/6/14.
//

#ifndef EX1_MIDDLEWARE_H
#define EX1_MIDDLEWARE_H

#include <vector>
#include <string>
#include "Request.h"
#include "Response.h"
#include <functional>


class Middleware{
public:
    virtual ~Middleware() = default;
    virtual void call(Request &req, Response &res) = 0;
};

namespace mw{


class StaticFileMiddleware: public Middleware {
public:
    void add_static_file_dir(const std::string& path);
    inline void set_file_extension_to_mimetype_mapping(const char *ext, const char *mime);

    void call(Request &req, Response &res) override;
private:
    std::vector<std::string> static_file_dirs_;
    std::unordered_map<std::string, std::string> file_extension_to_mimetype_map_;
};


}


#endif //EX1_MIDDLEWARE_H
