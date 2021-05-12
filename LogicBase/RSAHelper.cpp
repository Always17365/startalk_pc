//
// Created by may on 2018/8/21.
//

#include <openssl/bio.h>


#include "RSAHelper.h"
#include "../Platform/NavigationManager.h"

namespace QTalk {
    namespace utils {
        namespace rsa {
            int padding = RSA_PKCS1_PADDING;

            RSA *createRSA(unsigned char *key, int publickey) {
//                OpenSSL_add_all_algorithms();
                BIO *keybio = BIO_new(BIO_s_file());
                int nRet = static_cast<int>(BIO_read_filename(keybio, key));

                RSA *rsa = nullptr;
                if (keybio == nullptr) {
                    printf("Failed to create key BIO");
                    return 0;
                }
                if (publickey) {
                    int rsaEncodeType = NavigationManager::instance().getRsaEncodeType();
                    if (rsaEncodeType)
                        rsa = PEM_read_bio_RSA_PUBKEY(keybio, nullptr, nullptr, nullptr);
                    else
                        rsa = PEM_read_bio_RSAPublicKey(keybio, nullptr, nullptr, nullptr);
                } else {
                    rsa = PEM_read_bio_RSAPrivateKey(keybio, nullptr, nullptr, nullptr);
                }
                if (rsa == nullptr) {
                    std::string err;
                    printLastError(&err);
                    printf("Failed to create RSA");
                }
                BIO_free(keybio);

                return rsa;
            }

            int public_encrypt(unsigned char *data, int data_len, unsigned char *filepath, unsigned char *encrypted) {
                RSA *rsa = createRSA(filepath, 1);
                int result = -1;
                if (nullptr != rsa)
                    result = RSA_public_encrypt(data_len, data, encrypted, rsa, padding);
                RSA_free(rsa);
                return result;
            }

            int private_decrypt(unsigned char *enc_data, int data_len, unsigned char *filepath, unsigned char *decrypted) {
                RSA *rsa = createRSA(filepath, 0);
                int result = RSA_private_decrypt(data_len, enc_data, decrypted, rsa, padding);
                return result;
            }


            int private_encrypt(unsigned char *data, int data_len, unsigned char *filepath, unsigned char *encrypted) {
                RSA *rsa = createRSA(filepath, 0);
                int result = RSA_private_encrypt(data_len, data, encrypted, rsa, padding);
                return result;
            }

            int public_decrypt(unsigned char *enc_data, int data_len, unsigned char *filepath, unsigned char *decrypted) {
                RSA *rsa = createRSA(filepath, 1);
                int result = RSA_public_decrypt(data_len, enc_data, decrypted, rsa, padding);
                return result;
            }

            void printLastError(std::string *output) {
                char err[130];
                ERR_load_crypto_strings();
                ERR_error_string(ERR_get_error(), err);
                output->assign(err);
            }
        }
    }
}

