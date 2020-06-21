//
// Created by dodo on 2020/6/17.
//

#include "Buffer.h"

void Buffer::append(const char *data, size_t len) {
    if(writeable_size()  < len){
        buffer_.resize(buffer_.size() + len);
    }
    char *p = const_cast<char*>(begin());
    std::copy(data, data + len, p + write_index_);
    write_index_ += len;
}

const char *Buffer::find(const char *needle, size_t len) const {
    const char* end = begin() + write_index_;
    const char* p = std::search(peek(), end , needle, needle + len);
    return p == end ? nullptr : p;
}
