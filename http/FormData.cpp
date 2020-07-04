//
// Created by dodo on 2020/7/3.
//

#include "FormData.h"
#include <regex>

namespace ws {

bool http::MultipartFormDataParser::parse(Buffer &buffer) {
    static const std::regex re_content_type(R"(^Content-Type:\s*(.*?)\s*$)",
                                            std::regex_constants::icase);

    static const std::regex re_content_disposition(
            R"__(^Content-Disposition:\s*form-data;\s*name="(.*?)"(?:;\s*filename="(.*?)")?\s*$)__",
            std::regex_constants::icase);
    static const std::string dash_ = "--";
    static const std::string crlf_ = "\r\n";

    const bool parse_finished = true;
    const bool parse_unfinished = false;

    while (state_ != End) {
        switch (state_) {
            case Boundary: {
                auto pattern = dash_ + boundary_ + crlf_;
                if (pattern.size() > buffer.readable_size()) {
                    return parse_unfinished;
                }
                auto pos = buffer.find(pattern);
                if (pos != 0) {
                    return parse_finished;
                }
                buffer.consume(pattern.size());
                state_ = NewEntry;
                break;
            }
            case NewEntry: {
                // 清空上一个 entry 的信息，其实在 move 的时候已经清空了
                // form_item_ = FormDataItem();
                state_ = Header;
                break;
            }
            case Header: {
                auto pattern = crlf_;

                auto pos = buffer.find(pattern);
                while (pos != std::string::npos) {
                    // 空行，说明 header 解析完了
                    if (pos == 0) {
                        buffer.consume(crlf_.size());
                        state_ = Body;
                        break;
                    }
                    auto header = std::string(buffer.peek(), pos);
                    {
                        std::smatch m;
                        if (std::regex_match(header, m, re_content_type)) {
                            form_item_.content_type = m[1];
                        } else if (std::regex_match(header, m, re_content_disposition)) {
                            form_item_.name = m[1];
                            form_item_.filename = m[2];
                        }
                    }

                    buffer.consume(pos + crlf_.size());
                    pos = buffer.find(pattern);
                }
                if(state_ == Header){
                    return parse_unfinished;
                }else{
                    break;
                }
            }
            case Body: {
                {
                    auto pattern = crlf_ + dash_ + boundary_;
                    if (pattern.size() > buffer.readable_size()) {
                        return parse_unfinished;
                    }

                    auto pos = buffer.find(pattern);
                    if (pos != std::string::npos) {
                        form_item_.content.append(buffer.peek(), pos);
                        buffer.consume(pos + pattern.size());

                        // 一个 form entry 已经解析完了
                        form_data_.add(std::move(form_item_));
                        // 判断这是不是最后一个 entry
                        state_ = Boundary_end;
                    } else {
                        // 没有找到的时候，不能把当前缓冲区里的内容全部加到 content 里面，
                        // 因为 pattern 可以已经出现了一部分，只是没有完全出现而已
                        // 一种优化措施是把当前缓冲区的内容长度 - pattern.size() 如果比
                        // pattern.size() 大，那就读取那么多
                        size_t size = buffer.readable_size() - pattern.size();
                        size = std::max(size, 1lu);
                        form_item_.content.append(buffer.peek(), size);
                        buffer.consume(size);
                    }
                }
                break;
            }
            case Boundary_end: {
                LOG_DEBUG << "Boundary_end";
                if (crlf_.size() > buffer.readable_size()) {
                    return parse_unfinished;
                }
                // 以 crlf 结尾，说明后面还有 entry
                if (buffer.find(crlf_) == 0) {
                    buffer.consume(crlf_.size());
                    state_ = NewEntry;
                } else {
                    auto pattern = dash_ + crlf_;
                    if (pattern.size() > buffer.readable_size()) {
                        return parse_unfinished;
                    }
                    // 以  `--\r\n` 结尾结束了
                    if (buffer.find(pattern) == 0) {
                        buffer.consume(pattern.size());
                        is_valid_ = true;
                        state_ = End;
                    } else {
                        return parse_finished;
                    }
                }
                break;
            }
            case End:
                break;
        }
    }

    return parse_finished;
}

bool http::MultipartFormDataParser::parse(const std::string &s) {
    Buffer buffer;
    buffer.append(s.data(), s.size());
    parse(buffer);
    return true;
}


}