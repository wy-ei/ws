//
// Created by wangyu on 2020/6/15.
//

#include <stack>
#include <utility>
#include "path.h"
#include "str.h"
#include <algorithm>



namespace ws {

std::string path::ext(const std::string &path) {
    auto i = path.rfind('.');
    if (i == std::string::npos) {
        return "";
    }
    if (i > 0 && path[i - 1] == '.') {
        return "";
    }
    return path.substr(i + 1);
}


std::string path::join(const std::vector<std::string> &args) {
    if (args.empty()) return ".";

    std::string path;
    for (auto &arg : args) {
        if (arg.empty()) {
            continue;
        }
        if (path.empty()) {
            path = arg;
        } else {
            path += '/';
            path += arg;
        }
    }
    if (path.empty()) return ".";

    return path::normalize(path);
}

std::string path::normalize(const std::string &path) {
    if (path.empty()) return ".";

    bool is_absolute = path.front() == '/';
    bool is_dir = path.back() == '/';

    std::vector<std::string> stk;

    str::split(path, '/', [&stk](std::string s) {
        if (s == ".." && !stk.empty()) {
            stk.pop_back();
        } else if (s != "." && !s.empty()) {
            stk.push_back(std::move(s));
        }
    });

    if (stk.empty()) {
        return is_absolute ? "/" : "./";
    }

    std::string new_path = is_absolute ? "/" : "./";

    for (auto &s: stk) {
        new_path += s;
        new_path += '/';
    }

    if (!is_dir) {
        new_path.pop_back();
    }

    return new_path;
}


} // end namespace ws