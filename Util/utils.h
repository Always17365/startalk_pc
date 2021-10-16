//
// Created by may on 2018/8/16.
//

#ifndef STALK_IO_UTILS_H
#define STALK_IO_UTILS_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <stdexcept>
#include <string>
#include <functional>

#include "util_global.h"

namespace st {
class STALK_UTIL_EXPORT utils
{
public:
    /**
     * @brief string_to_hex
     * @param input
     * @param len
     * @return
     */
    static std::string string_to_hex(const std::string &input,
                                     size_t len);

    /**
     * @brief hex_to_string
     * @param input
     * @return
     */
    static std::string hex_to_string(const std::string &input);

    /**
     * @brief smallEndian
     * @return
     */
    static bool smallEndian();

    /**
     * @brief EndianIntConvertBToLittle
     * @param InputNum
     * @return
     */
    static uint32_t EndianIntConvertBToLittle(uint32_t InputNum);

    /**
     * @brief EndianIntConvertLToBig
     * @param InputNum
     * @return
     */
    static int32_t EndianIntConvertLToBig(int32_t InputNum);

    /**
     * @brief encodeWithGZip
     * @param input
     * @return
     */
    static std::string encodeWithGZip(std::string *input);

    /**
     * @brief decodeWithGZip
     * @param input
     * @return
     */
    static std::string decodeWithGZip(std::string *input);

    /**
     * @brief UrlEncode
     * @param str
     * @return
     */
    static std::string UrlEncode(const std::string &str);

    /**
     * @brief format
     * @param format
     * @return
     */
    static std::string format(const char *format, ...);

    /**
     * @brief getMessageId
     * @return
     */
    static std::string uuid();

    /**
     * @brief replaceAll
     * @param srcStr
     * @param matStr
     * @param dstStr
     * @return
     */
    static std::string replaceAll(const std::string &srcStr,
                                  char matStr,
                                  const std::string &dstStr);

    /**
     * @brief getStrMd5
     * @param src
     * @return
     */
    static std::string getStrMd5(const std::string &src);

    /**
     * @brief getFileMd5
     * @param filePath
     * @return
     */
    static std::string getFileMd5(const std::string &filePath);

    /**
     * @brief getFileDataMd5
     * @param data
     * @return
     */
    static std::string getFileDataMd5(const std::string *data);

    /**
     * @brief getFileSuffix
     * @param filePath
     * @return
     */
    static std::string getFileSuffix(const std::string &filePath);

    static std::string base64_encode(unsigned char const *, unsigned int len);
    static std::string base64_decode(std::string const &s);
};

}

#endif //STALK_IO_UTILS_H
