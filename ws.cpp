//
// Created by dodo on 2020/6/14.
//
#include "http/Server.h"
#include "http/Middleware.h"

int main() {
    Server server;

    auto sfm = std::make_shared<mw::StaticFileMiddleware>();

    sfm->add_static_file_dir("/mnt/c/Users/wy/work/code/notebook/_site/");
    server.use(sfm);

    server.use("GET", "/hello", [](Request &req, Response &res) {
        std::string text = "hello";
        res.set_header("cookie", "bbb=123");
        res.set_status(200);
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
