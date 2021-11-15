//
// Created by cc on 2019-02-25.
//

#ifndef STALK_V2_SYSTEMDEFINE_H
#define STALK_V2_SYSTEMDEFINE_H

#include <string>
#include <sstream>
#include <iostream>
#include "platform_global.h"

#define DB_VERSION 100014

#define GLOBAL_INTERNAL_VERSION 200015
#define GLOBAL_VERSION "build-2.0.015"

#ifndef _WINDOWS
    #define APPLICATION_VERSION "2.0 "  GLOBAL_VERSION
#else
    #ifdef WIN32
        #define APPLICATION_VERSION "2.0 Windows(x86) " GLOBAL_VERSION
    #else
        #define APPLICATION_VERSION "2.0 Windows(x64) " GLOBAL_VERSION
    #endif // WIN32
#endif // _WINDOWS

time_t PLATFORMSHARED_EXPORT build_time();

#endif //STALK_V2_SYSTEMDEFINE_H
