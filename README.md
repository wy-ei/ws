## ws - web server

### 功能介绍

这是一个基于 C++11 编写的基于 Reactor 模式的 Web 服务器，支持 GET/HEAD/POST 请求。支持中间件，使用中间件处理静态文件请求，支持了缓存控制。
支持自定义路由，可以编写代码来处理指定路径上的请求。支持长链接，支持超时自动断开。另外实现了一个异步的日志模块，用于调试。

## 示例

在用户接口上我模仿了 express 的设计，重载 use 方法，可以添加中间件、路由。

```c++
#include "http/HTTPServer.h"
#include "http/mw/StaticFileMiddleware.h"
#include "log/logging.h"

using namespace ws::http;

int main(int argc, char* argv[]) {
    // 启动异步日志模块，如果不启动，日志会打印到 stdout 上
    // ws::logging::start_async_backend(argv[0]);
    ws::logging::set_level(ws::logging::INFO);

    // 创建一个 http server，1 个线程做 accept，8 个线程处理请求
    HTTPServer server(/* thread num */8);

    // 添加一个中间件用来处理静态文件
    auto sfm = std::make_shared<StaticFileMiddleware>();
    sfm->add_static_file_dir("/Users/wangyu/code/blog/_site");
    server.use(sfm);

    // 处理 post 请求
    server.use("POST", "/form", [](Request &req, Response &res) {
        res.set_status(200);
        std::string text;
        if(req.is_multipart_form_data() && req.form_data.has("foo")){
            const FormDataItem& item = req.form_data.get("foo");

            std::ostringstream os;
            os << "name:" << item.name;
            os << "content-type:" << item.content_type;
            os << "content:" << item.content;
            text = os.str();
        }else{
            text = "<null>";
        }
        res.write(text);
        res.end();
    });

    // 处理 get 请求 /hello?name=world
    server.use("GET", "/hello", [](Request &req, Response &res) {
        std::string name;
        if(req.query.find("name") != req.query.end()){
            name = req.query["name"];
        }else{
            name = "world";
        }
        std::string text = "hello " + name;
        res.set_status(200);
        res.write(text);
        res.end();
    });

    // 在 8006 端口上启动 server
    server.listen("0.0.0.0", 8006);
}
```

## 编译与运行

项目中使用了 C++11 的语法，可以在 Linux 和 macOS 上运行，我使用的开发环境是 WSL Ubuntu 18.04 和 macOS 10.15。使用  gcc 7.5 和 clang 11 编译。

可以使用下面几行命令即可编译并运行，我是使用 CLion 来开发的。

```
$ cmake CMakeLists.txt
$ make
$ ./ws
```

## TODO

- [x] 断开超时未响应连接
- [ ] 支持路由的模式匹配，实现例如 `/user/:id/home` 这样的路由配置。 
- [x] 支持 POST 请求的常见格式
- [ ] 支持路由配置的模块化  

## 后记

在看 [APUE](https://book.douban.com/subject/1788421/) / [UNP](https://book.douban.com/subject/1500149/) / 
[《Linux多线程服务端编程》](https://book.douban.com/subject/20471211/) / [Linux高性能服务器编程](https://book.douban.com/subject/24722611/)
等书时，写了不少几十行～二三百行的代码，用来实践书中的内容。后来希望实现一个较大的项目，觉得 HTTP 服务器是不错的选择，这是我的初步实现。

开始采用了 `select` + 线程池的模型，主线程 `accept` 后，把处理请求的任务加入到线程池的任务队列中。
后来尝试基于 Reactor 模式来实现，折腾了几天后，已经可以跑了，但是代码比较混乱。后来读了陈硕写的 《Linux多线程服务端编程》，
了解了 Reactor 模式中的一些有用的抽象，比如 Channel、EventLoop 等，通读了 muduo 的大部分代码后，对我的代码进行了重构。

后来有了一台 Mac Book Pro 我就不想使用原来的 Windows 本了，于是在 Mac 上对 IO 多路复用进行了抽象，在 macOS 上使用 select 实现，
在 Linux 上使用 epoll 实现。这过程中我阅读了 Redis 中的[部分代码](https://github.com/redis/redis/blob/6.0/src/ae.h)，借鉴了其思想。
