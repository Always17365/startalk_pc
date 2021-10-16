//
// Created by may on 2018/5/4.
//

#include <sstream>
#include <iostream>
#include <string.h>
#include <future>
#include <deque>
#include "JID.h"
#include "utils.h"

using namespace std;

namespace st {
namespace entity {

void JID::innerParseUser(const string &input, const std::string &clientVersion, const std::string &platformStr,
                         int channel)
{

    const std::string &jid(input);

    size_t pos = jid.find('@');

    if (pos > 0 && pos <= jid.size()) {
        std::string node = jid.substr(0, pos);
        std::string remain = jid.substr(pos + 1, jid.size() - pos);

        pos = remain.find('/');
        std::string domain;
        std::string resource;

        if (remain.empty()) {
        } else {
            if (pos > 0 && pos <= remain.size()) {
                domain = remain.substr(0, pos);
                resource = remain.substr(pos + 1, remain.size() - pos);
            } else {
                domain = remain;
                std::ostringstream stream;
                stream << "V[" << clientVersion
                       << "]_P[" << platformStr
                       << "]_ID[" << st::utils::uuid()
                       << "]_C[" << channel
                       << "]_PB";
                resource = stream.str();
            }

            init(node, domain, resource, channel);
        }
    }
}

JID::JID(const std::string &jid, const string &clientVersion, const string &platformStr, int channel)
{
    innerParseUser(jid, clientVersion, platformStr, channel);
}


void JID::init(const std::string &node, const  std::string &domain,
               const std::string &resource, int channel)
{
    if (node.empty() || domain.empty()) {
        //throw "jid's parameter is illegal";
    }

    _domain = domain;
    _node = node;
    _resource = resource;
    _channel = channel;
}

std::string JID::basename()
{
    if (_node.empty() || _domain.empty()) {
        return std::string();
    }

    std::stringstream ss;
    ss << _node
       << "@"
       << _domain;
    return ss.str();
}

std::string JID::fullname()
{
    if (_resource.empty()) {
        return basename();
    }

    std::stringstream ss;
    ss << _node
       << "@"
       << _domain
       << "/"
       << _resource;
    return ss.str();
}

std::string &JID::username()
{
    return _node;
}

std::string &JID::domainname()
{
    return _domain;
}

string &JID::resources()
{
    return this->_resource;
}

}
}
