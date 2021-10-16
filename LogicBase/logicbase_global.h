//
// Created by cc on 2019-08-13.
//

#ifndef STALK_V2_LOGICBASE_GLOBAL_H
#define STALK_V2_LOGICBASE_GLOBAL_H

#ifdef _WINDOWS
#define LOGICBASE_EXPORT __declspec(dllexport)
#endif // _WINDOWS

#if (defined _MACOS) || (defined _LINUX)
#define LOGICBASE_EXPORT
#endif

#endif //STALK_V2_LOGICBASE_GLOBAL_H
