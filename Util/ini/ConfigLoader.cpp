//
// Created by may on 2018/11/7.
//

#include "ConfigLoader.h"
#include <QSettings>

//
// ini parser 回调，会初始化map
namespace st {

class ConfigLoaderPrivate
{
    friend class ConfigLoader;
    std::string _filename;
    QSettings  *_settings{nullptr};

public:
    bool load()
    {
        _settings = new QSettings(_filename.data(), QSettings::IniFormat);
        return _settings->status() == QSettings::NoError;
    }
};

ConfigLoader::ConfigLoader(const std::string &filePath)
    : _d(new ConfigLoaderPrivate)
{
    _d->_filename = filePath;
}

ConfigLoader::~ConfigLoader()
{
    delete _d;
}

// load ini 文件
bool ConfigLoader::load()
{
    return _d->load();
}

// 获取int值
int ConfigLoader::getInteger(const std::string &key)
{
    if (_d->_settings) {
        return _d->_settings->value(key.data()).toInt();
    }

    return 0;
}

//
// 获取string型值
std::string ConfigLoader::getString(const std::string &key)
{
    if (_d->_settings) {
        return _d->_settings->value(key.data()).toString().toStdString();
    }

    return "";
}

double ConfigLoader::getDouble(const std::string &key)
{
    if (_d->_settings) {
        return _d->_settings->value(key.data()).toDouble();
    }

    return 0.0;
}

float ConfigLoader::getFloat(const std::string &key)
{
    if (_d->_settings) {
        return _d->_settings->value(key.data()).toFloat();
    }

    return 0.0;
}

//
// 获取bool类型值
bool ConfigLoader::getBool(const std::string &key)
{
    if (_d->_settings) {
        return _d->_settings->value(key.data()).toBool();
    }

    return false;
}

//
// 设置bool类型值
void ConfigLoader::setBool(const std::string &key, bool value)
{
    if (_d->_settings) {
        _d->_settings->setValue(key.data(), value);
    }
}

//
// 设置string类型值
void ConfigLoader::setString(const std::string &key,
                             const std::string &value)
{
    if (_d->_settings) {
        _d->_settings->setValue(key.data(), value.data());
    }
}

//
// 设置int类型值
void ConfigLoader::setInteger(const std::string &key, int value)
{
    if (_d->_settings) {
        _d->_settings->setValue(key.data(), value);
    }
}

//
// 判断是否包含key
bool ConfigLoader::hasKey(const std::string &key)
{
    if (_d->_settings) {
        _d->_settings->contains(key.data());
    }

    return false;
}

//
// 删除节点
void ConfigLoader::remove(const std::string &key)
{
    if (_d->_settings) {
        _d->_settings->remove(key.data());
    }
}

//
// ini writer,配置写入。一次性全量写入
void ConfigLoader::saveConfig()
{

}

}

