﻿//
// Created by may on 2018/8/21.
//

#ifndef QTALK_RSAHELPER_H
#define QTALK_RSAHELPER_H

#include <string>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <stdio.h>
#include <string.h>

namespace QTalk {
    namespace utils {
        namespace rsa {

            int public_encrypt(unsigned char *data, int data_len, unsigned char *filepath, unsigned char *encrypted);
				
            int private_decrypt(unsigned char *enc_data, int data_len, unsigned char *filepath, unsigned char *decrypted);
				
            int private_encrypt(unsigned char *data, int data_len, unsigned char *filepath, unsigned char *encrypted);
				
            int public_decrypt(unsigned char *enc_data, int data_len, unsigned char *filepath, unsigned char *decrypted);

            void printLastError(std::string *output);
        }
    }
}


#endif //QTALK_RSAHELPER_H