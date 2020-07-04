//
// Created by wangyu on 2020/6/24.
//

#include "logging.h"
#include <chrono>
#include "../base/Date.h"



namespace ws {
namespace logging {

LEVEL logging_level = TRACE;

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

OutputCallback logging_output = default_output_callback;
FlushCallback logging_flush = default_flush_callback;

std::shared_ptr<Logging> p_logging;

using std::placeholders::_1;
using std::placeholders::_2;

Logger::Logger(const char *basename, int line, ws::logging::LEVEL level)
    : level_(level), basename_(basename), line_(line) {
        output_time();
        stream_ << std::this_thread::get_id();
        stream_ << LevelName[level];
    }

void Logger::output_time() {
    using namespace std::chrono;
    system_clock::time_point now = system_clock::now();
    long now_us = duration_cast<microseconds>(now.time_since_epoch()).count();
    time_t now_time = system_clock::to_time_t(now);

    char time_buffer[64];
    struct tm tm_time{};
    localtime_r(&now_time, &tm_time);
    size_t n = strftime(time_buffer, sizeof(time_buffer), "%Y%m%d %H:%M:%S", &tm_time);

    long us = now_us % 1000000;
    snprintf(&time_buffer[n], sizeof(time_buffer) - n, ".%06ld ", us);

    stream_ << time_buffer;
}

Logger::~Logger() {
    stream_ << " - " << basename_ << ':' << line_ << '\n';
    std::string s = stream_.str();
    logging_output(s.data(), s.size());
    if (level_ == FATAL) {
        logging_flush();
        abort();
    }
}

template<size_t SIZE>
ssize_t FixedBuffer<SIZE>::append(const char *data, size_t len) {
    if (writable_size() < len) {
        return -1;
    }
    memcpy(&buffer_[write_index_], data, len);
    write_index_ += len;
    return len;
}


void LogFile::roll_file() {
    if(fd_ != -1){
        close(fd_);
    }
    file_size_ = 0;
    std::string filename = build_filename();
    fd_ = ::open(filename.c_str(), O_CREAT | O_TRUNC | O_WRONLY);
    assert(fd_ > 0);
}

std::string LogFile::build_filename() {
    std::string filename;
    // 格式为： [program_name]-[time]-[host].log

    filename += program_name_;

    Date date;
    filename += date.to_locale_string("-%Y%m%d-%H%M%S-");

    char buffer[256];
    if (::gethostname(buffer, sizeof buffer) == 0){
        buffer[sizeof(buffer) - 1] = '\0';
        filename += buffer;
    }else{
        filename += "unknownhost";
    }

    filename += ".log";

    return filename;
}

void LogFile::write(const std::list<std::shared_ptr<Buffer>> &buffers) {
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



Logging::Logging(const std::string &program_name) {
    logging_output = std::bind(&Logging::append, this, _1, _2);
    logging_flush = std::bind(&Logging::flush, this);

    current_buffer_ = std::make_shared<Buffer>();
    file_.init(program_name);
    thread_ = std::make_shared<std::thread>(std::bind(&Logging::thread_func, this));
}

void Logging::append(const char *message, size_t len) {
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

void Logging::thread_func() {
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

void Logging::stop() {
    if(stop_ == true) {
        return;
    }
    logging_output = default_output_callback;
    logging_flush = default_flush_callback;
    stop_ = true;
    thread_->join();
    flush();
}

std::shared_ptr<Buffer> Logging::next_free_buffer() {
    if (free_buffers_.empty()) {
        return std::make_shared<Buffer>();
    } else {
        auto buffer = free_buffers_.front();
        free_buffers_.pop_front();
        return buffer;
    }
}

void Logging::flush() {
    if(current_buffer_->size()){
        full_buffers_.push_back(current_buffer_);
    }
    if(!full_buffers_.empty()){
        file_.write(full_buffers_);
    }
}


} // end namespace detail


std::shared_ptr<detail::Logging> p_logging;

void start_async_backend(const std::string &program_name) {
    static int n = 0;
    if(n != 0){
        return;
    }
    n = 1;
    p_logging = std::make_shared<detail::Logging>(program_name);
}

void set_level(ws::logging::LEVEL level) {
    logging_level = level;
}

void stop_async_backend() {
    if(detail::p_logging == nullptr){
        return;
    }else{
        detail::p_logging->stop();
    }
}


} // end namespace logging
} // end namespace ws


