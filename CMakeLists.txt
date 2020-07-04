cmake_minimum_required(VERSION 3.5)
project(ws)

set(CMAKE_CXX_STANDARD 11)

add_executable(ws http/HTTPServer.cpp utils/test.cpp http/HTTPServer.h net/comm.h base/ThreadPool.h http/Request.cpp http/Request.h http/Response.cpp http/Response.h http/Router.cpp http/Router.h http/imp.cpp http/imp.h ws.cpp http/Middleware.cpp http/Middleware.h utils/path.cpp utils/path.h net/Channel.cpp net/Channel.h net/EPoll.h net/Socket.h net/Socket.cpp net/Timeout.h base/Buffer.cpp base/Buffer.h net/TCPServer.cpp net/TCPServer.h net/EventLoop.cpp net/EventLoop.h net/EPoll.cpp http/Context.cpp http/Context.h net/Conn.cpp net/Conn.h test/socket-test.cpp utils/str.cpp utils/str.h log/logging.h test/log-test.cpp log/logging.cpp http/comm.h base/noncopyable.h utils/perf.h test/pref-test.cpp http/FormData.cpp http/FormData.h test/formdata-test.cpp base/except.h base/Date.cpp base/Date.h)


find_package(Threads REQUIRED)
target_link_libraries(ws Threads::Threads)
