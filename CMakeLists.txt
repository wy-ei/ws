cmake_minimum_required(VERSION 3.5)
project(ws)

set(CMAKE_CXX_STANDARD 14)

add_executable(ws http/Server.cpp utils/test.cpp http/Server.h net/comm.h base/ThreadPool.h http/Request.cpp http/Request.h http/Response.cpp http/Response.h http/Router.cpp http/Router.h http/imp.cpp http/imp.h ws.cpp http/Middleware.cpp http/Middleware.h utils/path.cpp utils/path.h net/Channel.cpp net/Channel.h net/EPoll.h net/Socket.h net/Socket.cpp base/Timeout.cpp base/Timeout.h base/Buffer.cpp base/Buffer.h net/TCPServer.cpp net/TCPServer.h net/EventLoop.cpp net/EventLoop.h net/EPoll.cpp http/Context.cpp http/Context.h test/Acceptor.cpp test/Acceptor.h net/Conn.cpp net/Conn.h test/socket-test.cpp utils/str.cpp utils/str.h log/logging.cpp log/logging.h)

find_package(Threads REQUIRED)
target_link_libraries(ws Threads::Threads)