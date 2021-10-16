//
// Created by cc on 2018/12/4.
//

#ifndef STALK_LOG_H
#define STALK_LOG_H

#include <string>
#include "util_global.h"

#include <iostream>
#include "Util/sformat.h"


#define __text_out(...) (SFormat(__VA_ARGS__).c_str())

#define LOGGER_LEVEL_INFO

#ifndef _WINDOWS

#define debug_log(fmt, ...) \
    do {   \
        st::logger::debug(SFormat(SFormat("{3}\n{0}({1}):{2}", __FILE__, __LINE__, __PRETTY_FUNCTION__, fmt).c_str(), ##__VA_ARGS__)); \
    } while (0)

#define info_log(fmt, ...) \
    do {   \
        st::logger::info(SFormat(SFormat("{3}\n{0}({1}):{2}", __FILE__, __LINE__, __PRETTY_FUNCTION__, fmt).c_str(), ##__VA_ARGS__)); \
    } while (0)

#define warn_log(fmt, ...) \
    do {   \
        st::logger::warning(SFormat(SFormat("{3}\n{0}({1}):{2}", __FILE__, __LINE__, __PRETTY_FUNCTION__, fmt).c_str(), ##__VA_ARGS__)); \
    } while (0)

#define error_log(fmt, ...) \
    do {   \
        st::logger::error(SFormat(SFormat("{3}\n{0}({1}):{2}", __FILE__, __LINE__, __PRETTY_FUNCTION__, fmt).c_str(), ##__VA_ARGS__)); \
    } while (0)

#else
#define debug_log(fmt, ...) \
    do {   \
        st::logger::debug(SFormat(SFormat("{3}\n{0}({1}):{2}", __FILE__, __LINE__, __FUNCTION__, fmt).c_str(), ##__VA_ARGS__)); \
    } while (0)

#define info_log(fmt, ...) \
    do {   \
        st::logger::info(SFormat(SFormat("{3}\n{0}({1}):{2}", __FILE__, __LINE__, __FUNCTION__, fmt).c_str(), ##__VA_ARGS__)); \
    } while (0)

#define warn_log(fmt, ...) \
    do {   \
        st::logger::warning(SFormat(SFormat("{3}\n{0}({1}):{2}", __FILE__, __LINE__, __FUNCTION__, fmt).c_str(), ##__VA_ARGS__)); \
    } while (0)


#define error_log(fmt, ...) \
    do {   \
        st::logger::error(SFormat(SFormat("{3}\n{0}({1}):{2}", __FILE__, __LINE__, __FUNCTION__, fmt).c_str(), ##__VA_ARGS__)); \
    } while (0)
#endif

/**
 * log 日志类
 */
namespace st {

namespace logger {

struct None {
};

template<typename First, typename Second>
struct Pair {
    First first;
    Second second;
};

template<typename List>
struct LogData {
    List list;
};

template<typename Begin, typename Value>
LogData<Pair<Begin, const Value &>>
                                 operator<<(LogData<Begin> begin, const Value &value)
{
    return {{begin.list, value}};
}

template<typename Begin, size_t n>
LogData<Pair<Begin, const char *>>
                                operator<<(LogData<Begin> begin, const char (&value)[n])
{
    return {{begin.list, value}};
}

inline void printList(std::ostream &os, None)
{
}


template<typename Begin, typename Last>
void printList(std::ostream &os, const Pair<Begin, Last> &data)
{
    printList(os, data.first);
    os << data.second;
}

template<typename List>
void log(const char *file, int line, const LogData<List> &data)
{
    std::cout << file << " (" << line << "): ";
    printList(std::cout, data.list);
    std::cout << "\n";
}

bool STALK_UTIL_EXPORT debug(const std::string &msg);

bool STALK_UTIL_EXPORT info(const std::string &msg);

bool STALK_UTIL_EXPORT warning(const std::string &msg);

bool STALK_UTIL_EXPORT error(const std::string &msg);

class log
{

public:
    template<class ...ARGS>
    static void info(const char *fmt, const ARGS &...args)
    {
        const auto tuple = std::forward_as_tuple(args...);
        std::string result = SFormatN<sizeof...(args)>::Format(fmt, tuple);
        st::logger::info(result);
    }

    template<class ...ARGS>
    static void warning(const char *fmt, const ARGS &...args)
    {
        const auto tuple = std::forward_as_tuple(args...);
        std::string result = SFormatN<sizeof...(args)>::Format(fmt, tuple);

    }

    template<class ...ARGS>
    static void error(const char *fmt, const ARGS &...args)
    {
        const auto tuple = std::forward_as_tuple(args...);
        std::string result = SFormatN<sizeof...(args)>::Format(fmt, tuple);
        st::logger::error(result);
    }
};
}
}
#endif //STALK_LOG_H
