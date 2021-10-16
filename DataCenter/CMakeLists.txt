project(DataCenter)

include_directories(.)

#
set(SOURCE_FILES
        NavigationManager.cpp
        NavigationManager.h
        Platform.cpp
        Platform.h
        platform_global.h
        dbPlatForm.cpp dbPlatForm.h
        AppSetting.cpp AppSetting.h)

if (APPLE)
    ADD_DEFINITIONS(-D_MACOS)
elseif (UNIX)
    ADD_DEFINITIONS(-D_LINUX)
else ()
    ADD_DEFINITIONS(-D_WINDOWS)
    ADD_DEFINITIONS(-DPLATFORM_LIB)
endif ()

set(LIBRARY_OUTPUT_PATH ../bin)
set(CMAKE_DEBUG_POSTFIX "d")



ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCE_FILES})

#添加链接库
target_link_libraries(${PROJECT_NAME}
        Util
        LogicManager)

