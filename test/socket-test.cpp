
#include "../utils/test.h"
#include "../net/Socket.h"
#include <thread>
#include <chrono>
using ws::TEST;
using ws::assert_true;

int socket_test(){
    TEST("socket", []{
        Socket server(AF_INET, SOCK_STREAM, 0);
        Socket client(AF_INET, SOCK_STREAM, 0);

        server.bind({"127.0.0.1", 8002});
        std::thread t([&]{
            server.listen();
            Socket conn = server.accept();
            while(true){
                char buf[100];
                int n = conn.recv(buf, 100);
                buf[n] = '\0';
                if(strncmp("quit", buf, 4) == 0){
                    break;
                }else{
                    conn.send(buf, n);
                }
            }
        });
        // make sure thread has started
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        assert_true(client.send("hello") == -1, "send before connect will fail");
        bool success = client.connect({"127.0.0.1", 8002});
        assert_true(success, "connect should success");
        int n = client.send("1234");
        assert_true(n == 4);

        char recvbuf[100];
        n = client.recv(recvbuf, sizeof(recvbuf));
        recvbuf[n] = '\0';
        assert_true(strncmp(recvbuf, "1234", 4) == 0);

        n = client.send("quit");
        t.join();
    });
}