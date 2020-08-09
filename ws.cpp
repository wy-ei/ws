//
// Created by wangyu on 2020/6/14.
//
#include "http/HTTPServer.h"
#include "http/mw/StaticFileMiddleware.h"
#include "log/logging.h"

using namespace ws::http;

int main(int argc, char* argv[]) {
    // 启动异步日志模块，如果不启动，日志会打印到 stdout 上
    // ws::logging::start_async_backend(argv[0]);
    ws::logging::set_level(ws::logging::INFO);

    // 创建一个 http server，用 8 个线程处理请求
    HTTPServer server(8);

    // 添加一个中间件用来处理静态文件
    auto sfm = std::make_shared<StaticFileMiddleware>();
    // 请根据自己的环境修改一下路径
    sfm->add_static_file_dir("/Users/wangyu/code/blog/_site");
    server.use(sfm);

    // 处理 post 请求
    server.use("POST", "/form", [](Request &req, Response &res) {
        res.set_status(200);
        std::string text;
        if(req.is_multipart_form_data() && req.form_data.has("foo")){
            const FormDataItem& item = req.form_data.get("foo");
            text = item.content;
        }else{
            text = "<null>";
        }
        res.write(text);
        res.end();
    });

    // 处理 get 请求 /hello?name=world
    server.use("GET", "/hello", [](Request &req, Response &res) {
        std::string name =  req.query["name"];
        std::string text = "hello " + name;

        res.set_status(200);
        res.write(text);
        res.end();
    });

    // 在 8006 端口上启动 server
    server.listen("0.0.0.0", 8006);
}
