//
// Created by may on 2018/8/16.
//
#include "utils.h"

#include <zlib.h>
#include <sstream>
#include <string.h>
#include <fstream>
#include <QCryptographicHash>
#include <QUuid>
#include <QFile>

using std::string;
using std::vector;
using std::ostringstream;
using std::istringstream;
using std::stringstream;

namespace st {

static const char *const hexTable = "0123456789ABCDEF";

string utils::string_to_hex(const string &input, size_t len)
{
    string output;
    output.reserve(2 * len);

    for (size_t i = 0; i < len; ++i) {
        const auto c = static_cast<const unsigned char>(input[i]);
        output.push_back(hexTable[c >> 4]); // NOLINT
        output.push_back(hexTable[c & 15]);
    }

    return output;
}

string utils::hex_to_string(const string &input)
{
    size_t len = input.length();

    if (len & 1) {
        throw std::invalid_argument("odd length");
    }

    string output;
    output.reserve(len / 2);

    for (size_t i = 0; i < len; i += 2) {
        char a = input[i];
        const char *p = std::lower_bound(hexTable, hexTable + 16, a);

        if (*p != a) {
            throw std::invalid_argument("not a hex digit");
        }

        char b = input[i + 1];
        const char *q = std::lower_bound(hexTable, hexTable + 16, b);

        if (*q != b) {
            throw std::invalid_argument("not a hex digit");
        }

        output.push_back(static_cast<char>(((p - hexTable) << 4) |
                                           (q - hexTable))); // NOLINT
    }

    return output;
}


string utils::format(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    auto len = static_cast<unsigned long>(vsnprintf(nullptr, 0, format, args));
    va_end(args);
    vector<char>
    vec(len + 1);
    va_start(args, format);
    vsnprintf(&vec[0], len + 1, format, args);
    va_end(args);
    return &vec[0];
}

bool utils::smallEndian()
{
    int n = 1;
    return *(char *) &n;
}

uint32_t utils::EndianIntConvertBToLittle(uint32_t InputNum)
{
    if (!smallEndian()) {
        char *p = (char *) &InputNum;
        uint32_t num, num1, num2, num3, num4;
        num1 = (uint32_t) (*p) << 24;
        num2 = ((uint32_t) * (p + 1)) << 16;
        num3 = ((uint32_t) * (p + 2)) << 8;
        num4 = ((uint32_t) * (p + 3));
        num = num1 + num2 + num3 + num4;
        return num;
    }

    return InputNum;
}

int32_t utils::EndianIntConvertLToBig(int32_t InputNum)
{
    if (smallEndian()) {
        char *p = (char *) &InputNum;
        char r[4];
        r[3] = p[0];
        r[2] = p[1];
        r[1] = p[2];
        r[0] = p[3];
        int32_t num;
        memcpy(&num, r, 4);
        return num;
    }

    return InputNum;
}

typedef struct _tagCompreessBuffer {
    char *p;
    uLong length;
} compressbuff;

compressbuff *compressbuff_new(int size)
{
    auto *buff = static_cast<compressbuff *>(malloc(sizeof(compressbuff)));
    buff->p = static_cast<char *>(malloc(sizeof(char) * size));
    buff->length = 0;
    return buff;
}

Bytef *compressbuff_pos(compressbuff *buf, int pos)
{
    return (Bytef *) (buf->p + pos);
}

void compressbuff_free(compressbuff *buff)
{
    free(buff->p);
    free(buff);
}

string utils::encodeWithGZip(string *input)
{
    if (input != nullptr && input->length() > 0) {
        int finished = 0;
        z_stream zStream;
        {
            // init
            memset(&zStream, 0, sizeof(zStream));
            zStream.zalloc = Z_NULL;
            zStream.zfree = Z_NULL;
            zStream.opaque = Z_NULL;
            zStream.avail_in = 0;
            zStream.next_in = 0;
            int windowsBits = 15;
            int GZIP_ENCODING = 16;
            int status = deflateInit2(&zStream,
                                      Z_DEFAULT_COMPRESSION,
                                      Z_DEFLATED,
                                      windowsBits | GZIP_ENCODING,
                                      8,
                                      Z_DEFAULT_STRATEGY);

            if (status != Z_OK) {
                return "";
            }
        }
        ostringstream ostringstream;
        uint32_t buflen = 16;
        int status = 0;
        zStream.next_in = (Bytef *) input->c_str();
        zStream.avail_in = (uInt) input->length();
        zStream.avail_out = 0;
        uLong totalLen = 0;

        while (zStream.avail_out == 0) {
            compressbuff *buf = compressbuff_new(buflen);
            zStream.next_out = compressbuff_pos(buf, 0);
            zStream.avail_out = buflen;
            status = deflate(&zStream, (finished == 0) ? Z_FINISH : Z_NO_FLUSH);

            if (status == Z_STREAM_END) {
                buf->length = zStream.total_out - totalLen;
                totalLen += buf->length;
                string s(buf->p, buf->length);
                ostringstream << s;
                compressbuff_free(buf);
                break;
            } else if (status != Z_OK) {
                compressbuff_free(buf);
                return "";
            }

            buf->length = buflen;
            totalLen += buf->length;
            string s(buf->p, buf->length);
            ostringstream << s;
            compressbuff_free(buf);
        }

        deflateEnd(&zStream);
        return ostringstream.str();
    }

    return "";
}

string utils::decodeWithGZip(string *input)
{
    if (input->length() > 0) {
        z_stream zStream;
        {
            //
            // init
            memset(&zStream, 0, sizeof(zStream));
            // Setup the inflate stream
            zStream.zalloc = Z_NULL;
            zStream.zfree = Z_NULL;
            zStream.opaque = Z_NULL;
            zStream.avail_in = 0;
            zStream.next_in = 0;
            int status = inflateInit2(&zStream, 16 + MAX_WBITS);

            if (status != Z_OK) {
                return "";
            }
        }
        ostringstream ostringstream;
        int status;
        int buflen = 16;
        zStream.next_in = (z_const
                           Bytef *) input->c_str();
        zStream.avail_in = (uInt) input->length();
        zStream.avail_out = 0;
        uLong totalLen = 0;

        while (zStream.avail_in != 0) {
            compressbuff *buf = compressbuff_new(buflen);
            zStream.next_out = compressbuff_pos(buf, 0);
            zStream.avail_out = static_cast<uInt>(buflen);
            status = inflate(&zStream, Z_NO_FLUSH);

            if (status != Z_OK && status != Z_STREAM_END) {
                compressbuff_free(buf);
                return "";
            }

            buf->length = buflen - zStream.avail_out;
            totalLen += buf->length;
            string s(buf->p, buf->length);
            ostringstream << s;
            compressbuff_free(buf);
        }

        inflateEnd(&zStream);
        return ostringstream.str();
    }

    return "";
}

unsigned char ToHex(unsigned char x)
{
    return  x > 9 ? x + 55 : x + 48;
}

string utils::UrlEncode(const string &str)
{
    string strTemp = "";
    size_t length = str.length();

    for (size_t i = 0; i < length; i++) {
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~')) {
            strTemp += str[i];
        } else if (str[i] == ' ') {
            strTemp += "+";
        } else {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }

    return strTemp;
}

vector<string> split(const string &s, char delimiter)
{
    vector<string> dst;
    string temp;
    istringstream tokenStream(s);

    while (getline(tokenStream, temp, delimiter)) {
        dst.push_back(temp);
    }

    return dst;
}

string utils::replaceAll(const string &srcStr, char matStr,
                         const string &dstStr)
{
    auto splitRet = split(srcStr, matStr);
    stringstream ss;
    unsigned long index = 0;

    for (const auto &str : splitRet) {
        ss << str;

        if (index != splitRet.size() - 1) {
            ss << dstStr;
        }
    }

    return ss.str();
}

string utils::uuid()
{
    QUuid id = QUuid::createUuid();
    return id.toString(QUuid::Id128).toStdString();
}

string utils::getStrMd5(const string &src)
{
    return QCryptographicHash::hash(src.data(),
                                    QCryptographicHash::Md5).toHex().toStdString();
}

string utils::getFileMd5(const string &filePath)
{
    QFile tmpF(filePath.data());

    if (tmpF.open(QIODevice::ReadOnly)) {
        QByteArray data = QCryptographicHash::hash(tmpF.readAll(),
                                                   QCryptographicHash::Md5);
        tmpF.close();
        return data.toHex().toStdString();
    }

    return "";
}

string utils::getFileDataMd5(const string *data)
{
    return QCryptographicHash::hash(data->data(),
                                    QCryptographicHash::Md5).toHex().toStdString();
}

string utils::getFileSuffix(const string &url)
{
    string suffix;
    unsigned long t = url.find_last_of('.');

    if (t != string::npos) {
        suffix = url.substr(t + 1);
        t = suffix.find_first_of('?');

        if (t != string::npos) {
            suffix = suffix.substr(0, t);
        }

        t = suffix.find_first_of('&');

        if (t != string::npos) {
            suffix = suffix.substr(0, t);
        }
    }

    return suffix;
}


static const string base64_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";


static inline bool is_base64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

string utils::base64_encode(unsigned char const *bytes_to_encode,
                            unsigned int in_len)
{
    string ret;
    int i = 0;
    int j = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];

    while (in_len--) {
        char_array_3[i++] = *(bytes_to_encode++);

        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >>
                                                                 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >>
                                                                 6);
            char_array_4[3] = char_array_3[2] & 0x3f;

            for (i = 0; (i < 4) ; i++) {
                ret += base64_chars[char_array_4[i]];
            }

            i = 0;
        }
    }

    if (i) {
        for (j = i; j < 3; j++) {
            char_array_3[j] = '\0';
        }

        char_array_4[0] = ( char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >>
                                                             4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >>
                                                             6);

        for (j = 0; (j < i + 1); j++) {
            ret += base64_chars[char_array_4[j]];
        }

        while ((i++ < 3)) {
            ret += '=';
        }

    }

    return ret;

}

string utils::base64_decode(string const &encoded_string)
{
    int in_len = encoded_string.size();
    int i = 0, j = 0, in_ = 0;

    unsigned char char_array_4[4], char_array_3[3];
    string ret;

    while (in_len-- && ( encoded_string[in_] != '=')
           && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;

        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            }

            char_array_3[0] = ( char_array_4[0] << 2 ) +
                              ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) +
                              ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) +
                              char_array_4[3];

            for (i = 0; (i < 3); i++) {
                ret += char_array_3[i];
            }

            i = 0;
        }
    }

    if (i) {
        for (j = 0; j < i; j++) {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4)
                          + ((char_array_4[2] & 0x3c) >> 2);

        for (j = 0; (j < i - 1); j++) {
            ret += char_array_3[j];
        }
    }

    return ret;
}

};
