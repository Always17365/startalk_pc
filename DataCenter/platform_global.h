#ifndef PLATFORM_GLOBAL_H
#define PLATFORM_GLOBAL_H

#ifdef _WINDOWS
#ifndef BUILD_STATIC
    #if defined(PLATFORM_LIB)
        #define PLATFORMSHARED_EXPORT __declspec(dllexport)
    #else
        #define PLATFORMSHARED_EXPORT __declspec(dllimport)
    #endif
#else
    #define PLATFORMSHARED_EXPORT
#endif
#else
#define PLATFORMSHARED_EXPORT
#endif

#endif // PLATFORM_GLOBAL_H
