//
// Created by dodo on 2020/6/21.
//

#include "str.h"


namespace ws{

void str::split(const std::string &s, char sep, const std::function<void(std::string)> &fn) {
    auto i = s.begin();
    while(i != s.end()){
        if(*i == sep) i++;
        auto j = std::find(i, s.end(), sep);
        if(std::distance(i, j) > 0){
            std::string span(i, j);
            fn(std::move(span));
        }
        i = j;
    }
}

}