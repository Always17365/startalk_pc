//
// Created by cc on 2019-08-13.
//

#ifndef QTALK_V2_LOGICBASE_GLOBAL_H
#define QTALK_V2_LOGICBASE_GLOBAL_H

#ifdef _WINDOWS
#define LOGICBASE_EXPORT __declspec(dllexport)
#endif // _WINDOWS

#if (defined _MACOS) || (defined _LINUX)
#define LOGICBASE_EXPORT
#endif

#endif //QTALK_V2_LOGICBASE_GLOBAL_H
