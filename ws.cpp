//
// Created by wangyu on 2020/6/14.
//
#include "http/HTTPServer.h"
#include "http/mw/StaticFileMiddleware.h"
#include "log/logging.h"

using namespace ws::http;

int main(int argc, char* argv[]) {
    //ws::logging::start_async_backend(argv[0]);
    ws::logging::set_level(ws::logging::INFO);

    LOG_DEBUG << strerror(4);
    LOG_DEBUG << strerror(6);

    HTTPServer server;

    auto sfm = std::make_shared<StaticFileMiddleware>();
    sfm->add_static_file_dir("/Users/wangyu/code/blog/_site");
    server.use(sfm);

    server.use("POST", "/form", [](Request &req, Response &res) {
        res.set_status(200);
        std::string text;
        if(req.is_multipart_form_data() && req.form_data.has("foo")){
            text = req.form_data.get("foo").content;
        }else{
            text = "<null>";
        }
        res.write(text);
        res.end();
    });

    server.use("GET", "/world", [](Request &req, Response &res) {
        std::string text = "world";
        res.set_status(200);
        res.write(text);
        res.end();
    });

    server.listen("0.0.0.0", 8006);
}


