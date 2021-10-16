//
// Created by cc on 2018/12/4.
//

#include <sstream>
#include "Log.h"
#include <QDebug>

namespace st {

namespace logger {

bool debug(const std::string &msg)
{
    //    qDebug() << msg.data();
    return true;
}

bool info(const std::string &msg)
{
    qInfo() << msg.data();
    return true;
}

bool warning(const std::string &msg)
{
    qWarning() << msg.data();
    return true;
}

bool error(const std::string &msg)
{
    qCritical() << msg.data();
    return true;
}

}
}
