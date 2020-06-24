//
// Created by wangyu on 2020/6/14.
//
#include <vector>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "Middleware.h"
#include "imp.h"
#include "../log/logging.h"
#include "../utils/path.h"



void mw::StaticFileMiddleware::call(Request &req, Response &res) {
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
            const char* content_type = imp::find_content_type(path, file_extension_to_mimetype_map_);
            res.set_header("Content-Type", content_type);

            // TODO: handle not modify

            res.set_status(200);
            res.set_header("Content-Length", std::to_string(st.st_size));

            int fd = open(path.c_str(), O_RDONLY);
            if(fd < 0) {
                LOG_ERROR << "can't open fill: " << strerror(errno);
                res.set_status(500);
                res.write("can't open fill");
                res.end();
                return;
            }else{
                char buff[8000];
                int n = 0;
                while ((n = read(fd, buff, 4000)) > 0){
                    res.write(buff, n);
                }
                res.end();
                return;
            }

        }
    }
}

void mw::StaticFileMiddleware::add_static_file_dir(const std::string &path) {
    std::string abs_path = ws::path::normalize(path);
    LOG_DEBUG << "add path:" << abs_path;
    static_file_dirs_.push_back(std::move(abs_path));
}

void mw::StaticFileMiddleware::set_file_extension_to_mimetype_mapping(const char *ext, const char *mime) {
    file_extension_to_mimetype_map_[ext] = mime;
}

