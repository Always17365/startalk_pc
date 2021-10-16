//
// Created by may on 2018/8/17.
//
#include <sstream>
#include "QTalkSecurity.h"
#include <time.h>
#include <string.h>
#include "RSAHelper.h"
#include "DataCenter/Platform.h"
#include "Util/nJson/nJson.h"
#include "Util/utils.h"
#include "Util/Log.h"
#include "DataCenter/NavigationManager.h"

using namespace st::entity;

namespace st {
namespace Security {
std::string pwd_key_plain(const std::string &user, const std::string &password,
                          int loginType)
{
    if (loginType == 1) {
        std::ostringstream plantText;
        plantText << '\0'
                  << user
                  << '\0'
                  << password;
        return utils::base64_encode((unsigned char *)plantText.str().data(),
                                    plantText.str().size());
    } else {

        struct tm *timeinfo = nullptr;
        time_t rawtime;
        char buffer[80];

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        memset(buffer, 0, 80);

        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
        (std::string(buffer));

        nJson obj;
        obj["p"] = password;
        obj["u"] = user;
        obj["a"] = "testapp";
        obj["d"] = buffer;
        std::string jsonStr = obj.dump();


        unsigned char encrypted[4098] = {};
        unsigned char decrypted[4098] = {};

        memset(encrypted, 0, 4098);
        memset(decrypted, 0, 4098);

        std::string strPath = DC.getAppdataRoamingUserPath();
        strPath += "/" + NavigationManager::instance().getPubkey();

        int encrypted_length = st::rsa::public_encrypt((
                                                           unsigned char *) jsonStr.c_str(),
                                                       static_cast<int>(jsonStr.length()),
                                                       (unsigned char *) strPath.c_str(),
                                                       encrypted);

        if (encrypted_length == -1) {
            std::string err;
            st::rsa::printLastError(&err);
            return "";
        } else {
            std::string str((char *) encrypted, encrypted_length);
            std::string encryped = utils::base64_encode(encrypted, encrypted_length);

            info_log("===== {0} {1}", str, encryped);

            const int totalLength = (user.length() + encryped.length() + 2);

            char buf[2048] = {};
            memset(buf, 0, static_cast<size_t>(totalLength));

            memcpy(buf + 1, user.c_str(), user.length());
            memcpy(buf + 2 + user.length(), encryped.c_str(), encryped.length());

            return utils::base64_encode(reinterpret_cast<const unsigned char *>(buf),
                                        totalLength);
        }
    }
}

std::string chatRsaEncrypt(const std::string &value)
{
    std::string key = st::utils::getStrMd5(value);
    unsigned char encrypted[4098] = {};
    memset(encrypted, 0, 4098);
    std::string strPath = DC.getAppdataRoamingUserPath();
    strPath += "/" + NavigationManager::instance().getPubkey();

    int encrypted_length =
        st::rsa::public_encrypt((unsigned char *) key.c_str(),
                                static_cast<int>(key.length()),
                                (unsigned char *) strPath.c_str(),
                                encrypted);

    if (encrypted_length == -1) {
        std::string err;
        st::rsa::printLastError(&err);
        return "";
    } else {
        return utils::base64_encode(encrypted, encrypted_length);
    }
}

std::string normalRsaEncrypt(const std::string &value)
{
    unsigned char encrypted[4098] = {};
    memset(encrypted, 0, 4098);
    std::string strPath = DC.getAppdataRoamingUserPath();
    strPath += "/" + NavigationManager::instance().getPubkey();

    int encrypted_length = st::rsa::public_encrypt((unsigned char *) value.c_str(),
                                                   static_cast<int>(value.length()),
                                                   (unsigned char *) strPath.c_str(),
                                                   encrypted);

    if (encrypted_length == -1) {
        std::string err;
        st::rsa::printLastError(&err);
        return "";
    } else {
        return utils::base64_encode(encrypted, encrypted_length);
    }
}

}
}
