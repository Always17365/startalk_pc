//
// Created by cc on 2019-01-27.
//

#ifndef STALK_V2_WEBSERVICE_GLOBAL_H
#define STALK_V2_WEBSERVICE_GLOBAL_H
#include <QtCore/qglobal.h>

#if defined(WEBSERVICE_LIBRARY)
#  define WEBSERVICE_EXPORT Q_DECL_EXPORT
#else
#  define WEBSERVICE_EXPORT Q_DECL_IMPORT
#endif

#endif //STALK_V2_WEBSERVICE_GLOBAL_H
