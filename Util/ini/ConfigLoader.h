//
// Created by may on 2018/11/7.
//

#ifndef STALK_V2_CONFIGLOADER_H
#define STALK_V2_CONFIGLOADER_H


#include <string>
#include <map>
#include "Util/util_global.h"

namespace st {

class ConfigLoaderPrivate;

class STALK_UTIL_EXPORT ConfigLoader
{
public:
    explicit ConfigLoader(const std::string &filePath);
    ~ConfigLoader();

public:
    bool load();

    void saveConfig();

public:
    int getInteger(const std::string &key);

    std::string getString(const std::string &key);

    bool getBool(const std::string &key);

    double getDouble(const std::string &key);

    float getFloat(const std::string &key);

public:
    bool hasKey(const std::string &key);

    void remove(const std::string &key);

    void setInteger(const std::string &key, int value);

    void setBool(const std::string &key, bool value);

    void setString(const std::string &key, const std::string &value);

private:
    ConfigLoaderPrivate *_d{nullptr};
};

} // namespace st


#endif //STALK_V2_CONFIGLOADER_H
