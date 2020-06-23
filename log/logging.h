//
// Created by wangyu on 2020/6/21.
//

#ifndef WS_LOGGING_H
#define WS_LOGGING_H

#include <cstdarg>
#include <ostream>
#include <sstream>
#include <cstring>
#include <list>
#include <thread>
#include <functional>
#include <ctime>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <cassert>
#include <memory>
#include <atomic>

namespace ws {
namespace logging {


enum LEVEL {
    TRACE, DEBUG, INFO, WARN, ERROR, FATAL
};

namespace detail {

const char *LevelName[] = {
        " TRACE: ",
        " DEBUG: ",
        " INFO: ",
        " WARN: ",
        " ERROR: ",
        " FATAL: "
};

inline
void default_output_callback(const char *data, size_t len) {
    fwrite(data, 1, len, stdout);
}

inline
void default_flush_callback() {
    fflush(stdout);
}

using OutputCallback = std::function<void(const char *message, size_t len)>;
OutputCallback logging_output = default_output_callback;

using FlushCallback = std::function<void()>;
FlushCallback logging_flush = default_flush_callback;

LEVEL logging_level = TRACE;



using std::placeholders::_1;
using std::placeholders::_2;


// 日志系统的前端主要构件，收集用户的日志内容，格式化后传给后端来保存
class Logger {
public:
    Logger(const char *basename, int line, LEVEL level)
            : level_(level), basename_(basename), line_(line) {

        output_time();
        stream_ << std::this_thread::get_id();
        stream_ << LevelName[level];
    }

    void output_time() {
        // TODO ms
        char time_buffer[64];
        struct tm tm_time{};
        time_t now = time(nullptr);
        localtime_r(&now, &tm_time);
        size_t n = strftime(time_buffer, sizeof(time_buffer), "%Y%m%d %H:%M:%S", &tm_time);

        int ms = 0;
        int len = snprintf(&time_buffer[n], sizeof(time_buffer) - n, ".%06d ", ms);

        stream_ << time_buffer;
    }

    template<typename V>
    Logger &operator<<(const V &value) {
        stream_ << value;
        return *this;
    }

    ~Logger() {
        stream_ << " - " << basename_ << ':' << line_ << '\n';
        std::string s = stream_.str();
        logging_output(s.data(), s.size());
        if (level_ == FATAL) {
            logging_flush();
            abort();
        }
    }
private:
    std::ostringstream stream_;
    LEVEL level_;
    const char *basename_;
    int line_;
};

// 下面为日志系统的后端部分，负责接收前端传来的日志内容，并存储到磁盘上
// 主要思路是使用多个缓冲区接收前端传来的日志，然后在另一个线程中每隔三秒刷新一次，把内容刷到磁盘上去


// 固定大小的 buffer，用于缓存日志数据
template<size_t SIZE>
class FixedBuffer {
public:
    ssize_t append(const char *data, size_t len) {
        if (writable_size() < len) {
            return -1;
        }
        memcpy(&buffer_[write_index_], data, len);
        write_index_ += len;
        return len;
    }

    size_t writable_size() { return SIZE - write_index_; }
    size_t size() { return write_index_; }
    char *data() { return buffer_; }
    void reset() { write_index_ = 0; }

private:
    char buffer_[SIZE]{};
    size_t write_index_{0};
};

static const int kBufferSize = 4096;
using Buffer = FixedBuffer<kBufferSize>;


// 用于管理日志文件，向日志文件中写入 buffer
class LogFile {
public:
    void init(const std::string &program_name) {
        program_name_ = program_name;
        roll_file();
    }

    void roll_file(){
        if(fd_ != -1){
            close(fd_);
        }
        file_size_ = 0;
        std::string filename = build_filename();
        fd_ = ::open(filename.c_str(), O_CREAT | O_TRUNC | O_WRONLY);
        assert(fd_ > 0);
    }

    std::string build_filename(){
        std::string filename;
        // 格式为： [program_name]-[time]-[host].log

        filename += program_name_;

        char buffer[256];
        struct tm tm_time{};
        time_t now = time(nullptr);
        localtime_r(&now, &tm_time);
        size_t n = strftime(buffer, sizeof(buffer), "-%Y%m%d-%H%M%S-", &tm_time);

        filename += buffer;

        if (::gethostname(buffer, sizeof buffer) == 0){
            buffer[sizeof(buffer) - 1] = '\0';
            filename += buffer;
        }else{
            filename += "unknownhost";
        }

        filename += ".log";

        return filename;
    }

    void write(const std::list<std::shared_ptr<Buffer>> &buffers) {
        std::unique_lock<std::mutex> lock(mutex_);

        iovec vec[buffers.size()];
        int i = 0;
        for (auto &buffer: buffers) {
            vec[i].iov_base = (void *) buffer->data();
            vec[i].iov_len = buffer->size();
            i++;
        }
        ssize_t n = writev(fd_, vec, buffers.size());
        file_size_ += n;
        // 超过 1GB 后切换文件
        if(file_size_ > 1024*1024*1024){
            roll_file();
        }
    }

private:
    std::mutex mutex_;
    std::string program_name_;
    int fd_ { -1 };
    //int rolled_times {0};
    ssize_t file_size_ { 0 };
};


// log 的后端部分，用于接收前端传来的日志，在合适的时候把内容交给 LogFile 写入硬盘
class Logging {
public:
    explicit Logging(const std::string& program_name) {
        logging_output = std::bind(&Logging::append, this, _1, _2);
        logging_flush = std::bind(&Logging::flush, this);

        current_buffer_ = std::make_shared<Buffer>();
        file_.init(program_name);
        thread_ = std::make_shared<std::thread>(std::bind(&Logging::thread_func, this));
    }

    void stop() {
        if(stop_ == true) {
            return;
        }
        logging_output = default_output_callback;
        logging_flush = default_flush_callback;
        stop_ = true;
        thread_->join();
        flush();
    }

    ~Logging() {
        stop();
    }
private:
    void append(const char *message, size_t len) {
        /**
         * 写入 current_buffer_ 如果已经满了将其加入 free_buffers_ 尾部
         * 取 free_buffers_ 中下一个 buffer 作为新的 current_buffer_
         */
        std::unique_lock<std::mutex> lock(mutex_);

        if (current_buffer_->writable_size() >= len) {
            current_buffer_->append(message, len);
        } else {
            full_buffers_.push_back(current_buffer_);
            current_buffer_ = next_free_buffer();
            current_buffer_->append(message, len);
            cond_.notify_one();
        }
    }

    std::shared_ptr<Buffer> next_free_buffer() {
        if (free_buffers_.empty()) {
            return std::make_shared<Buffer>();
        } else {
            auto buffer = free_buffers_.front();
            free_buffers_.pop_front();
            return buffer;
        }
    }

    void flush(){
        if(current_buffer_->size()){
            full_buffers_.push_back(current_buffer_);
        }
        if(!full_buffers_.empty()){
            file_.write(full_buffers_);
        }
    }

    // 接收条件变量的通知或者每隔 flush_interval_ 秒就把现在已经写入的数据保存到磁盘上
    void thread_func() {
        while(!stop_){
            std::list<std::shared_ptr<Buffer>> buffers_to_write;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (!full_buffers_.empty()) {
                    buffers_to_write.swap(full_buffers_);
                }else{
                    cond_.wait_for(lock, std::chrono::seconds(flush_interval_));
                }
                if (current_buffer_->size() > 0) {
                    buffers_to_write.push_back(current_buffer_);
                    current_buffer_ = next_free_buffer();
                }

                if (buffers_to_write.empty()) {
                    continue;
                }
            }

            if (buffers_to_write.size() > 20) {
                std::cerr << "warning: ws::logging - log too frequency\n";
            }

            file_.write(buffers_to_write);

            while (buffers_to_write.size() > 5) {
                buffers_to_write.pop_front();
            }

            for (auto &buffer: buffers_to_write) {
                buffer->reset();
            }

            {
                std::unique_lock<std::mutex> lock(mutex_);
                free_buffers_.splice(free_buffers_.end(), buffers_to_write);
            }
        }
    }

private:
    std::mutex mutex_;
    std::condition_variable cond_;
    std::list<std::shared_ptr<Buffer>> full_buffers_;
    std::list<std::shared_ptr<Buffer>> free_buffers_;
    std::shared_ptr<Buffer> current_buffer_;

    int flush_interval_{3};
    LogFile file_;
    std::shared_ptr<std::thread> thread_;
    std::atomic<bool> stop_{false};
};

std::shared_ptr<Logging> p_logging;

} // end namespace detail



// 下面是暴露给用户的接口

// 启动后端，默认前端会把内容写到标准输出
inline void init(const std::string& program_name){
    static int n = 0;
    if(n != 0){
        return;
    }
    n = 1;
    detail::p_logging = std::make_shared<detail::Logging>(program_name);
}

inline void set_level(LEVEL level){
    detail::logging_level = level;
}

// 停止后端，此后日志会写入到标准输出
inline void stop(){
    if(detail::p_logging == nullptr){
        return;
    }else{
        detail::p_logging->stop();
    }
}

} // end namespace logging
} // end namespace ws


/*
 * 用法如下：
 *
 * LOG_INFO << "hello world";
 */

#define LOG_TRACE if(ws::logging::detail::logging_level <=ws::logging::TRACE )  ws::logging::detail::Logger(__FILE__, __LINE__, ws::logging::TRACE)
#define LOG_DEBUG if(ws::logging::detail::logging_level <=ws::logging::DEBUG ) ws::logging::detail::Logger(__FILE__, __LINE__, ws::logging::DEBUG)
#define LOG_INFO if(ws::logging::detail::logging_level <=ws::logging::INFO ) ws::logging::detail::Logger(__FILE__, __LINE__, ws::logging::INFO)
#define LOG_WARN ws::logging::detail::Logger(__FILE__, __LINE__, ws::logging::WARN)
#define LOG_ERROR ws::logging::detail::Logger(__FILE__, __LINE__, ws::logging::ERROR)
#define LOG_FATAL ws::logging::detail::Logger(__FILE__, __LINE__, ws::logging::FATAL)

// TODO 替换掉原来使用的 debug 函数
//void debug(const char *fmt, ...);

#endif //WS_LOGGING_H
