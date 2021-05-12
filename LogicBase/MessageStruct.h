#ifndef _MESSAGESTRUCT_H_
#define _MESSAGESTRUCT_H_

#include <map>
#include <vector>

struct StStringHeader
{
	std::map<std::string, std::string>  params;
	std::string                         key;
	std::string                         value;
	int                                 definedKey;
};

struct StMessageBody
{
	std::vector<StStringHeader>  headers;
	std::string                  value;
};

#endif // _MESSAGESTRUCT_H_