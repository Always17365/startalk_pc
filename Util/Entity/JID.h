//
// Created by may on 2018/5/4.
//

#ifndef WIDGET_JID_H
#define WIDGET_JID_H

#include <string>
#include "../util_global.h"

namespace st {
namespace entity {

class STALK_UTIL_EXPORT JID
{

public:
    explicit JID(const std::string &jid, const std::string &clientVersion = "",
                 const std::string &platformStr = "", int channel = 1);

public:
    std::string fullname();
    std::string basename();
    std::string &username();
    std::string &resources();
    std::string &domainname();

private:
    void init(const std::string &node, const  std::string &domain,
              const std::string &resource, int channel);

    void innerParseUser(const std::string &input,
                        const std::string &clientVersion,
                        const std::string &platformStr, int channel);

private:
    std::string _node;
    std::string _domain;
    std::string _resource;
    int         _channel{};
};
}

}


#endif //WIDGET_JID_H
