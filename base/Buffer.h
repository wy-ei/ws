//
// Created by dodo on 2020/6/17.
//

#ifndef WS_BUFFER_H
#define WS_BUFFER_H

#include <vector>
#include <algorithm>
#include <cstring>
#include <cassert>
#include "../log/logging.h"

class Buffer {
public:
    static const size_t k_initial_size = 1024;
    static const size_t k_prepend_size = 8;

    explicit Buffer(size_t initial_size = k_initial_size):
        buffer_(k_prepend_size + initial_size),
        read_index_(k_prepend_size),
        write_index_(k_prepend_size){

        assert(writeable_size() == initial_size);
        assert(readable_size() == 0);
        assert(prepend_size() == k_prepend_size);
    }

    void swap(Buffer& rhs){
        buffer_.swap(rhs.buffer_);
        read_index_ = rhs.read_index_;
        write_index_ = rhs.write_index_;
    }

    size_t readable_size() const{
        return write_index_ - read_index_;
    }

    size_t writeable_size() const{
        return buffer_.size() - write_index_;
    }

    bool consume(size_t n){
        if(readable_size() >= n){
            read_index_ += n;
            return true;
        }
        return false;
    }

    size_t read(char* p, size_t size){
        size = std::min(size, readable_size());
        if(size > 0){
            memcpy(p, peek(), size);
            read_index_ += size;
        }
        return size;
    }

    bool unread(size_t size){
        if(read_index_ - k_prepend_size >= size){
            read_index_ -= size;
            return true;
        }
        return false;
    }

    size_t prepend_size() const{
        return read_index_;
    }

    const char* peek() const{
        return buffer_.data() + read_index_;
    }

    const char* find(const char* needle, size_t len) const;

    void append(const char* data, size_t len);

private:
    const char* begin() const{
        return buffer_.data();
    }

    std::vector<char> buffer_;
    size_t read_index_;
    size_t write_index_;
};


#endif //WS_BUFFER_H
