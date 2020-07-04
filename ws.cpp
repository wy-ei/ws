//
// Created by wangyu on 2020/6/14.
//
#include "http/HTTPServer.h"
#include "http/Middleware.h"
#include "log/logging.h"

using namespace ws::http;

int main(int argc, char* argv[]) {
    ws::logging::start_async_backend(argv[0]);
    ws::logging::set_level(ws::logging::INFO);

    HTTPServer server;

    auto sfm = std::make_shared<mw::StaticFileMiddleware>();
    sfm->add_static_file_dir("/mnt/c/Users/dodo/work/code/webserver/test/");
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

    server.listen("127.0.0.1", 8006);
}
