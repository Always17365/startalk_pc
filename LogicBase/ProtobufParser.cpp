//
// Created by may on 2018/8/15.
//

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <sstream>
#include "ProtobufParser.h"
#include "ParseFailedException.h"
#include "Util/utils.h"

namespace st {
namespace Protocol {

void tea_encrypt(uint32_t *v, uint32_t *k)
{
    //printf("%.8x,%.8x\n",v[0],v[1]);
    uint32_t v0 = static_cast<uint32_t>(utils::EndianIntConvertLToBig(v[0])),
             v1 = static_cast<uint32_t>(utils::EndianIntConvertLToBig(v[1])),
             sum = 0, i;
    /* set up */
    uint32_t delta = 0x9e3779b9;                     /* a key schedule constant */
    uint32_t k0 = utils::EndianIntConvertBToLittle(k[0]),
             k1 = utils::EndianIntConvertBToLittle(
                      k[1]), k2 = utils::EndianIntConvertBToLittle(k[2]),
                             k3 = utils::EndianIntConvertBToLittle(k[3]);   /* cache key */

    for (i = 0; i < 32; i++) {                       /* basic cycle start */
        sum += delta;
        v0 += ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
        v1 += ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
    }

    /* end cycle */
    v[0] = static_cast<uint32_t>(utils::EndianIntConvertLToBig(v0));
    v[1] = static_cast<uint32_t>(utils::EndianIntConvertLToBig(v1));
    //printf("r %.8x,%.8x\n",v[0],v[1]);
}

void tea_decrypt(uint32_t *v, uint32_t *k)
{
    uint32_t v0 = static_cast<uint32_t>(utils::EndianIntConvertLToBig(
                                            v[0])), v1 = static_cast<uint32_t>(utils::EndianIntConvertLToBig(
                                                             v[1])), sum = 0xC6EF3720, i;  /* set up */
    uint32_t delta = 0x9e3779b9;                     /* a key schedule constant */
    uint32_t k0 = utils::EndianIntConvertBToLittle(k[0]),
             k1 = utils::EndianIntConvertBToLittle(k[1]),
             k2 = utils::EndianIntConvertBToLittle(k[2]),
             k3 = utils::EndianIntConvertBToLittle(k[3]);   /* cache key */

    for (i = 0; i < 32; i++) {                         /* basic cycle start */
        v1 -= ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
        v0 -= ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
        sum -= delta;
    }                                              /* end cycle */

    v[0] = static_cast<uint32_t>(utils::EndianIntConvertLToBig(v0));
    v[1] = static_cast<uint32_t>(utils::EndianIntConvertLToBig(v1));
}

static unsigned
scan_varint(unsigned len, const uint8_t *data)
{
    unsigned i;

    if (len > 10) {
        len = 10;
    }

    for (i = 0; i < len; i++)
        if ((data[i] & 0x80) == 0) {
            break;
        }

    if (i == len) {
        return 0;
    }

    return i + 1;
}

unsigned scan__varint(unsigned len, const uint8_t *data)
{
    return scan_varint(len, data);
}

size_t computeUInt32SizeNoTag(uint32_t value)
{
    if ((value & (0xffffffff << 7)) == 0) {
        return 1;
    }

    if ((value & (0xffffffff << 14)) == 0) {
        return 2;
    }

    if ((value & (0xffffffff << 21)) == 0) {
        return 3;
    }

    if ((value & (0xffffffff << 28)) == 0) {
        return 4;
    }

    return 5;
}

std::string uint32__pack(uint32_t value)
{
    size_t size = computeUInt32SizeNoTag(value);

    const unsigned bufLength = 32;
    unsigned char buffer[bufLength];

    memset(buffer, 0, bufLength);

    google::protobuf::io::ArrayOutputStream arrayOutput(buffer, bufLength);
    google::protobuf::io::CodedOutputStream codedOutput(&arrayOutput);
    codedOutput.WriteVarint32(value);

    std::ostringstream ostringstream;
    ostringstream << buffer;

    return ostringstream.str();
}

uint32_t parse__uint32(std::string *string)
{
    google::protobuf::io::CodedInputStream stream(
        reinterpret_cast<const google::protobuf::uint8 *>(string->c_str()),
        static_cast<int>(string->length()));
    uint32_t result;
    bool ok = stream.ReadVarint32(&result);
    return ok ? result : 0;
}

uint32_t parse__uint32(uint8_t *data, int lengh)
{
    google::protobuf::io::CodedInputStream stream(data, lengh);
    uint32_t result;
    bool ok = stream.ReadVarint32(&result);
    return ok ? result : 0;
}

bool parseMessageWithHeader(ProtoHeader *header, ProtoMessage *message,
                            uint32_t tea_key[4])
{

    if (header) {
        if (header->has_options()) {
            //
            // 0x01 是压缩，0x05 是 TEA
            if (header->options() == 0x01) {
                std::string msg = header->message();

                std::string output = utils::decodeWithGZip(&msg);

                if (output.length() > 0) {
                    message->ParseFromString(output);
                    return message->IsInitialized();
                } else {
                    throw st::Excetpion::ParseFailedException("unGZip failed.");
                }
            } else if (header->options() == 0x05) {
                int i = 0;

                while (header->message().length() - i >= 2 * (sizeof(uint32_t))) {
                    tea_decrypt((uint32_t *) ((uint8_t *) header->message().c_str() + i), tea_key);
                    i += 2 * sizeof(uint32_t);
                }

                message->ParseFromString(header->message());
                return message->IsInitialized();
            } else if (header->options() == 0x00) {
                message->ParseFromString(header->message());
                return message->IsInitialized();
            }
        } else {
            //
            // 木有选项，那奏是没压缩的ProtoMessage

            if (header->has_message()) {
                message->ParseFromString(header->message());
                return true;
            }
        }
    }

    return false;
}

bool parseMessageWithHeader(ProtoHeader *header, ProtoMessage *message)
{
    uint32_t tea_key[4] = {0, 0, 0, 0};
    return parseMessageWithHeader(header, message, tea_key);
}

ProtobufParser::ProtobufParser()
{
    tea_key[0] = 0x00;
    tea_key[1] = 0x00;
    tea_key[2] = 0x00;
    tea_key[3] = 0x00;

    resetParser();
}

ProtobufParser *ProtobufParser::newParser()
{
    return nullptr;
}

void ProtobufParser::resetParser()
{
    buff.clear();
    pos = 0;
    header_size = 0;
}

bool ProtobufParser::parseOneObject(ProtoMessage *message, const uint8_t *msg,
                                    size_t len)
{
    if (msg != nullptr && len > 0) {
        std::string input((const char *) msg, len);
        buff << input;

        unsigned varintcount = 0;
        int message_count = 0;

        do {
            if (header_size > 0) {
                //
                // 可以parse 一次了
                if (buff.length >= header_size + pos) {
                    std::string headerData = buff.getDataWithRange(pos, header_size);
                    ProtoHeader header;
                    header.ParseFromArray(headerData.c_str(), header_size);

                    if (header.IsInitialized()) {
                        parseMessageWithHeader(&header, message);

                        if (message->IsInitialized()) {
                            message_count++;
                            pos += header_size;
                            header_size = 0;

                            if (pos == buff.length) {
                                pos = 0;
                                buff.clear();
                            }

                            return true;
                        }
                    }
                } else {
                    break;
                }
            }

            if (pos == buff.length) {
                resetParser();
                break;
            }

            int remain = buff.length - pos;

            if (remain >= 1) {
                int size = 0;
                int count = remain > 3 ? 3 : remain;
                int i = 1;

                for (i = 1; i <= count; i++) {

                    /* code */ //整数最多10位
                    varintcount = scan__varint(i,
                                               reinterpret_cast<const uint8_t *>(buff.bytes() + pos));

                    if (varintcount > 0) {
                        int endPos = buff.length - pos > 10 ? 10 : buff.length - pos;
                        std::string value = buff.getDataWithRange(pos, endPos);
                        header_size = parse__uint32(&value);
                        pos += i;
                        break;
                    }
                }
            } else {
                varintcount = 0;
                break;
            }
        } while (varintcount > 0);
    } else {
        return false;
    }

    return false;
}

bool ProtobufParser::parseAllObjects(std::vector<ProtoMessage> objects,
                                     const uint8_t *msg, size_t len)
{
    return false;
}

std::string ProtobufParser::buildPackageForProtoMessage(
    const ProtoMessage *message)
{
    if (message) {

        long len = message->message().length();

        if (len > 200) {
            ProtoHeader header;
            header.set_options(0x01);

            std::string buffer = message->SerializeAsString();

            std::string encodeBuff = utils::encodeWithGZip(&buffer);

            header.set_message(encodeBuff);

            std::string headerData = header.SerializeAsString();

            size_t headerlen = headerData.length();
            std::string packLenData = uint32__pack(headerlen);

            int size = packLenData.length();

            std::ostringstream ostringstream;
            ostringstream << packLenData;
            ostringstream << headerData;
            return ostringstream.str();
        } else {
            //
            // 使用tea 加扰
            ProtoHeader header;
            header.set_options(0x05);
            int i = 0;

            std::string messageData = message->SerializeAsString();

            while ((messageData.length() - i) >= 2 * (sizeof(uint32_t))) {
                tea_encrypt((uint32_t *) ((uint8_t *) messageData.c_str() + i), tea_key);
                i += 2 * sizeof(uint32_t);
            }

            header.set_message(messageData);

            std::string headerData = header.SerializeAsString();

            size_t headerlen = headerData.length();

            std::string packLenData = uint32__pack(headerlen);

            std::ostringstream ostringstream;
            ostringstream << packLenData;
            ostringstream << headerData;
            return ostringstream.str();
        }
    }
}
}
}
