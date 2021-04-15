#include <string>
#include <gtest/gtest.h>

#include "net/Socket.h"
#include <thread>
#include <chrono>


using namespace ws::net;



TEST(SocketTests, socket){
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

        EXPECT_EQ(client.send("hello"), -1);
        bool success = client.connect({"127.0.0.1", 8002});
        EXPECT_TRUE(success);
        int n = client.send("1234");
        EXPECT_TRUE(n == 4);

        char recvbuf[100];
        n = client.recv(recvbuf, sizeof(recvbuf));
        recvbuf[n] = '\0';
        EXPECT_TRUE(strncmp(recvbuf, "1234", 4) == 0);

        n = client.send("quit");
        t.join();
}


int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}