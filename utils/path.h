//
// Created by dodo on 2020/6/15.
//

#ifndef _WLIB_PATH_H_
#define _WLIB_PATH_H_

#include <string>
#include <vector>

namespace ws {
namespace path {

std::string ext(const std::string &path);

std::string join(const std::vector<std::string>& args);

std::string normalize(const std::string &path);

}
}


#endif //_WLIB_PATH_H_
