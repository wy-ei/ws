//
// Created by dodo on 2020/7/4.
//

#ifndef WS_EXCEPT_H
#define WS_EXCEPT_H

#include <exception>
#include <string>
#include <utility>

class Key_Error : public std::exception
{
public:
    explicit Key_Error(std::string what): message_(std::move(what)){}
    ~Key_Error() noexcept override = default;

    const char* what() const noexcept override{
        return message_.c_str();
    }

private:
    std::string message_;
};

#endif //WS_EXCEPT_H
