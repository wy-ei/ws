## ws - web server

在看 [APUE](https://book.douban.com/subject/1788421/) 和 [UNP](https://book.douban.com/subject/1500149/) 的
时候写了不少短小的代码片段，但是不成体系。最近萌生了自己实现一个 web 服务器的想法，这是我的初步实现。

开始采用了 `select` + 线程池的模型，主线程 accept 后，把处理请求的任务加入到线程池的任务队列中。
后来尝试实现 Reactor 模式，根据自己的理解折腾了几天，大体实现了基本框架。后来读了陈硕写的 《linux多线程服务端编程》，
了解了 Reactor 模式中的一些有用的抽象，比如 Channel、EventLoop 等，之后我对代码进行了抽象，目前模块已经比较清晰了。


## 示例

在用户接口上我模仿了 express 的设计，重载 use 方法，可以添加中间件、用户处理函数。

```c++
#include "http/HTTPServer.h"
#include "http/Middleware.h"
using namespace ws::http;

int main() {
    HTTPServer server;

    auto sfm = std::make_shared<mw::StaticFileMiddleware>();

    sfm->add_static_file_dir("/mnt/c/Users/dodo/work/code/notebook/_site/");
    server.use(sfm);

    server.use("GET", "/hello", [](Request &req, Response &res) {
        std::string text = "hello";
        res.set_header("cookie", "bbb=123");
        res.set_status(200);
        res.write(text);
        res.end();
    });

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

    server.listen("127.0.0.1", 8006);
}
```

## 编译与运行

开发环境为 WSL - Ubuntu 18.04 - C++11，用下面几行命令即可编译并运行

```
$ cmake CMakeLists.txt
$ make
$ ./ws
```

## TODO

- [x] 断开超时未响应连接
- [ ] 支持路由的模式匹配
- [x] 支持 POST 请求的常见格式