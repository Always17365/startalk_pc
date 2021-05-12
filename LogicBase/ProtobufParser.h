//
// Created by may on 2018/8/15.
//

#ifndef LIBEVENTTEST_PROTOBUFPARSER_H
#define LIBEVENTTEST_PROTOBUFPARSER_H

#include <cstdint>
#include <vector>
#include "MemoryStream.h"
#include "Protobuf/message.pb.h"

namespace QTalk
{
    namespace Protocol 
	{
        std::string uint32__pack(uint32_t value);

        class ProtobufParser 
		{
        public:
            ProtobufParser();

            static ProtobufParser *newParser();

            bool parseOneObject(ProtoMessage *message, const uint8_t *msg, size_t len);

            bool parseAllObjects(std::vector<ProtoMessage> objects, const uint8_t *msg, size_t len);

            std::string buildPackageForProtoMessage(const ProtoMessage *pMessage);

        private:
            uint32_t tea_key[4];
            QTalk::utils::MemoryStream buff;
            int header_size;
            int pos;

            void resetParser();
        };
    }
}

#endif //LIBEVENTTEST_PROTOBUFPARSER_H
