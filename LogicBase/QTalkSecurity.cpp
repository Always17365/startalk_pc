//
// Created by may on 2018/8/17.
//
#include <sstream>
#include "QTalkSecurity.h"
#include <time.h>
#include <string.h>
#include "RSAHelper.h"
#include "../Platform/Platform.h"
#include "../QtUtil/nJson/nJson.h"
#include "../QtUtil/lib/Base64/base64.h"
#include "../Platform/NavigationManager.h"
#include "../QtUtil/lib/Md5/Md5.h"

using namespace QTalk::Entity;

namespace QTalk {
    namespace Security {
        std::string pwd_key_plain(const std::string &user, const std::string &password, int loginType) {

            if (loginType == 1)
            {
                std::ostringstream plantText;
                plantText << '\0'
                          << user
                          << '\0'
                          << password;
                return base64_encode((unsigned char*)plantText.str().data(), plantText.str().size());
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

                std::string strPath = PLAT.getAppdataRoamingUserPath();
                strPath += "/" + NavigationManager::instance().getPubkey();

                int encrypted_length = QTalk::utils::rsa::public_encrypt((unsigned char *) jsonStr.c_str(),
                                                                         static_cast<int>(jsonStr.length()),
                                                                         (unsigned char *) strPath.c_str(),
                                                                         encrypted);

                if (encrypted_length == -1) {
                    std::string err;
                    QTalk::utils::rsa::printLastError(&err);
                    return "";
                } else {
                    std::string str((char *) encrypted, encrypted_length);
                    std::string encryped = base64_encode(encrypted, encrypted_length);
                    const int totalLength = (user.length() + encryped.length() + 2);

                    char buf[2048] = {};
                    memset(buf, 0, static_cast<size_t>(totalLength));

                    memcpy(buf + 1, user.c_str(), user.length());
                    memcpy(buf + 2 + user.length(), encryped.c_str(), encryped.length());

                    return base64_encode(reinterpret_cast<const unsigned char *>(buf), totalLength);
                }
            }
        }

        std::string chatRsaEncrypt(const std::string &value){
            std::string key = MD5(value).toString();
            unsigned char encrypted[4098] = {};
            memset(encrypted, 0, 4098);
            std::string strPath = PLAT.getAppdataRoamingUserPath();
            strPath += "/" + NavigationManager::instance().getPubkey();

            int encrypted_length = QTalk::utils::rsa::public_encrypt((unsigned char *) key.c_str(),
                                                                     static_cast<int>(key.length()),
                                                                     (unsigned char *) strPath.c_str(),
                                                                     encrypted);
            if (encrypted_length == -1) {
                std::string err;
                QTalk::utils::rsa::printLastError(&err);
                return "";
            } else {
                return base64_encode(encrypted, encrypted_length);
            }
        }

        std::string normalRsaEncrypt(const std::string &value){
            unsigned char encrypted[4098] = {};
            memset(encrypted, 0, 4098);
            std::string strPath = PLAT.getAppdataRoamingUserPath();
            strPath += "/" + NavigationManager::instance().getPubkey();

            int encrypted_length = QTalk::utils::rsa::public_encrypt((unsigned char *) value.c_str(),
                                                                     static_cast<int>(value.length()),
                                                                     (unsigned char *) strPath.c_str(),
                                                                     encrypted);
            if (encrypted_length == -1) {
                std::string err;
                QTalk::utils::rsa::printLastError(&err);
                return "";
            } else {
                return base64_encode(encrypted, encrypted_length);
            }
        }

    }
}
