#include "../utils/test.h"
#include "../log/logging.h"
#include "../base/ThreadPool.h"
#include <atomic>

using ws::TEST;
using ws::assert_true;

using namespace ws::base;

int logging_test(){
//    ws::logging::set_level(ws::logging::INFO);

    TEST("ss", []{
        std::atomic<int> lines{0};
        ThreadPool pool(8);
        for(int i=0;i<8;i++){
            pool.enqueue([i, &lines](){
                for(int j = 0;j < 100; j++){
                    LOG_INFO << "thread-" << lines << " <" << j << '>';
                    lines++;
                }
            });
        }
        for(int i=0;i<8;i++){
            pool.enqueue([i, &lines](){
                for(int j = 0;j < 100; j++){
                    LOG_INFO << "thread-" << lines << " <" << i << '>';
                    lines++;
                }
            });
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        pool.shutdown();
    });
}

//int main(int argc, char* argv[]){
//    ws::logging::init(argv[0]);
//
//    logging_test();
//
//}