//
// Created by wangyu on 2020/6/29.
//

#ifndef WS_NONCOPYABLE_H
#define WS_NONCOPYABLE_H

class noncopyable {
protected:
    noncopyable() = default;
    ~noncopyable() = default;
private:
    noncopyable(const noncopyable &) = delete;
    const noncopyable &operator=(const noncopyable &) = delete;
};

#endif //WS_NONCOPYABLE_H
