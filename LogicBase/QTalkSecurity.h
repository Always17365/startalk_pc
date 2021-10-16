//
// Created by may on 2018/8/17.
//

#ifndef QTALKSECURITY_H
#define QTALKSECURITY_H


#include <string>
#include "Util/Entity/JID.h"

namespace st {
namespace Security {
std::string  pwd_key_plain(const std::string &user, const std::string &password,
                           int loginType);
std::string  chatRsaEncrypt(const std::string &value);
std::string  normalRsaEncrypt(const std::string &value);
}
}


#endif //QTALKSECURITY_H
