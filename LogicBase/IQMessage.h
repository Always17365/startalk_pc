#ifndef _IQMESSAGE_H_
#define _IQMESSAGE_H_

#include "MessageStruct.h"
#include "EventBus/Event.hpp"

class IQMessageEvt : public Event
{
public:
	std::string    strNamespace;
	std::string    key;
	std::string    value;
	std::string    messageId;
	StStringHeader header;
	StMessageBody  body;
	long long      receivedTime;
	long long      transferTime;
	std::vector<StStringHeader> headers;
	std::vector<StMessageBody>  bodys;
	int                         definedKey;

};

#endif//_IQMESSAGE_H_
