//
// Created by may on 2018/8/13.
//

#include <sstream>
#include <future>
#include "ProtobufStream.h"
#include "../QtUtil/Utils/utils.h"
#include "pb2json.h"
#include "../QtUtil/Utils/Log.h"

namespace QTalk {
    namespace Protocol {

        ProtobufStream::ProtobufStream() {
            _pSocket = QTalk::Socket::buildNewSocket();
            _pSocket->setDelegate(this);
        }

        ProtobufStream::~ProtobufStream() {
            delete _pSocket;
            _pSocket = nullptr;
        }

        void ProtobufStream::onDataReceived(const char *msg, size_t len) {
            ProtoMessage message;
            bool succeed = _parser.parseOneObject(&message, reinterpret_cast<const uint8_t *>(msg), len);
            if (succeed) {
//                std::string strMsg = message.SerializeAsString();
//                std::string hexBuff = QTalk::utils::string_to_hex(strMsg, strMsg.size());
//                std::cout << "recv data: " << hexBuff << std::endl;
//                info_log("recv data: {}", QTalk::toJson(&message));
                _delegate->onMessageReceived(&message);
            }
        }

        void ProtobufStream::onSocketConnected() {
            std::thread([this]() {
                if (this->_delegate != nullptr) {
                    _delegate->onSocketConnected();
                }
            }).detach();
        }

        void ProtobufStream::onSocketConnectFailed() {
            if (this->_delegate != nullptr) {
                _delegate->onSocketConnectFailed();
            }
        }


        void ProtobufStream::onSocketDisConnected() {
            if (this->_delegate != nullptr) {
                _delegate->onSocketDisConnected();
            }
        }

        void ProtobufStream::sendProtobufMessage(const ProtoMessage *message) {
            if (message == nullptr)
                return;
//            info_log("send data: {}", QTalk::toJson(message));
//            std::string hexBuff = QTalk::utils::string_to_hex(data, data.size());
            try {
                std::string data = _parser.buildPackageForProtoMessage(message);
                _pSocket->sendMessage(data);
            }
            catch (const std::exception& e)
            {
                error_log("exception {}", e.what());
            }
        }

        void ProtobufStream::onTlsStarted() {
            if (_delegate != nullptr) {
                _delegate->onTlsStarted();
            }
        }

        void ProtobufStream::setStreamDelegate(ProtobufStreamDelegate *delegate) {
            this->_delegate = delegate;
        }
    }
}



