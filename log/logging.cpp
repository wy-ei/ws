//
// Created by dodo on 2020/6/21.
//
#include <cstdio>
#include <cstdarg>
#include "logging.h"


void debug(const char* fmt, ...){
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
}