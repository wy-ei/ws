//
// Created by wangyu on 2020/6/14.
//
#include <vector>
#include <fcntl.h>
#include <sys/stat.h>

#include "StaticFileMiddleware.h"
#include "../../log/logging.h"
#include "../../utils/path.h"
#include "../Response.h"
#include "../Request.h"
#include "../../base/Date.h"

namespace ws{
namespace http{
using namespace net;

void StaticFileMiddleware::call(Request &req, Response &res) {
    if(res.finished()){
        return;
    }
    if(req.method != "HEAD" && req.method != "GET"){
        return;
    }
    for(const auto& base_dir: static_file_dirs_){
        std::string relative_path = ws::path::normalize(req.path);
        std::string path = ws::path::join({base_dir, relative_path});

        struct stat st{};
        if(path.back() == '/'){
            path += "index.html";
        }
        if(stat(path.c_str(), &st) < 0){
            continue;
        }
        // if is file
        if(S_ISREG(st.st_mode)){
#ifdef __APPLE__
            Date file_modified_date(st.st_mtimespec.tv_sec);
#else
            Date file_modified_date(st.st_mtim.tv_sec);
#endif
            if(req.has_header("If-Modified-Since")){
                std::string modified_since_str = req.get_header_value("If-Modified-Since");
                Date last_modified_date = Date::from_utc_string(modified_since_str);

                if(file_modified_date <= last_modified_date){
                    res.set_status(304);
                    res.end();
                    return;
                }
            }

            const char* content_type = find_content_type(path, file_extension_to_mimetype_map_);
            res.set_header("Content-Type", content_type);
            res.set_status(200);
            res.set_header("Content-Length", std::to_string(st.st_size));

            int fd = open(path.c_str(), O_RDONLY);
            if(fd < 0) {
                LOG_ERROR << "can't open fill: " << strerror(errno);
                res.set_status(403);
                res.write("Forbidden");
                res.end();
            }else{
                std::string cache_control = "private, max-age=36000, no-cache";
                res.set_header("Cache-Control", cache_control);
                res.set_header("Last-Modified", file_modified_date.to_utc_string());
                // 可以使用 sendfile 来优化
                char buff[8000];
                int n = 0;
                while ((n = read(fd, buff, 8000)) > 0){
                    res.write(buff, n);
                }
                res.end();
            }
            close(fd);
            return;
        }
    }
}

void StaticFileMiddleware::add_static_file_dir(const std::string &path) {
    std::string abs_path = ws::path::normalize(path);
    LOG_DEBUG << "add path:" << abs_path;
    static_file_dirs_.push_back(std::move(abs_path));
    auto it = std::find(static_file_dirs_.begin(), static_file_dirs_.end(), abs_path);
    if(it != static_file_dirs_.end()){
        LOG_WARN << "duplicate dir: " << abs_path;
    }else{
        static_file_dirs_.push_back(abs_path);
    }
}

void StaticFileMiddleware::set_file_extension_to_mimetype_mapping(const char *ext, const char *mime) {
    file_extension_to_mimetype_map_[ext] = mime;
}


const char* find_content_type(const std::string &path,
                              const std::unordered_map<std::string, std::string> &user_defined_dict) {
    int i = path.rfind('.');
    if(i == std::string::npos){
        return nullptr;
    }
    std::string ext = path.substr(i + 1);

    auto it = user_defined_dict.find(ext);
    if (it != user_defined_dict.end()) {
        return it->second.c_str();
    }

    if (ext == "txt") {
        return "text/plain";
    } else if (ext == "html" || ext == "htm") {
        return "text/html";
    } else if (ext == "css") {
        return "text/css";
    } else if (ext == "jpeg" || ext == "jpg") {
        return "image/jpg";
    } else if (ext == "png") {
        return "image/png";
    } else if (ext == "gif") {
        return "image/gif";
    } else if (ext == "svg") {
        return "image/svg+xml";
    } else if (ext == "ico") {
        return "image/x-icon";
    } else if (ext == "json") {
        return "application/json";
    } else if (ext == "pdf") {
        return "application/pdf";
    } else if (ext == "js") {
        return "application/javascript";
    } else if (ext == "wasm") {
        return "application/wasm";
    } else if (ext == "xml") {
        return "application/xml";
    } else if (ext == "xhtml") {
        return "application/xhtml+xml";
    }
    return "text/plain";
}


} // end namespace http
} // namespace ws