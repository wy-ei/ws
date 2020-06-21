//
// Created by dodo on 2020/6/21.
//

#ifndef _WS_STR_H_
#define _WS_STR_H_

#include <algorithm>
#include <string>
#include <functional>

namespace ws{
namespace str{

void split(const std::string& s, char sep, const std::function<void(std::string)>& fn);

template <typename Fn>
void split(const char* beg, const char* end, char sep, Fn fn){
    auto i = beg;
    while(i != end){
        if(*i == sep) i++;
        auto j = std::find(i, end, sep);
        if(std::distance(i, j) > 0){
            fn(i, j);
        }
        i = j;
    }
}




} // end namespace str
}// end namespace ws


#endif // _WS_STR_H_
