//
// Created by 王钰 on 2020/8/9.
//

#ifndef WS_STATICFILEMIDDLEWARE_H
#define WS_STATICFILEMIDDLEWARE_H

#include <string>
#include <vector>
#include "http/mw/Middleware.h"

namespace ws{
namespace http{


const char *find_content_type(const std::string &, const std::unordered_map<std::string, std::string> &);


class StaticFileMiddleware: public Middleware {
public:
    void add_static_file_dir(const std::string& path);
    inline void set_file_extension_to_mimetype_mapping(const char *ext, const char *mime);

    void call(Request &req, Response &res) override;
private:
    std::vector<std::string> static_file_dirs_;
    std::unordered_map<std::string, std::string> file_extension_to_mimetype_map_;
};


} // end namespace http
} // namespace ws

#endif //WS_STATICFILEMIDDLEWARE_H
