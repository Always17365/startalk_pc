project(Util)

find_package(Qt5 COMPONENTS Widgets Xml)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
include_directories(./lib)

if (APPLE)
    ADD_DEFINITIONS(-D_MACOS)
elseif (UNIX)
    ADD_DEFINITIONS(-D_LINUX)
    link_libraries(uuid)
else ()
    ADD_DEFINITIONS(-D_WINDOWS)
    ADD_DEFINITIONS(-DZLIB_WINAPI)
endif ()

set(SOURCE_FILES
    utils.cpp
    utils.h
    Log.h
    Log.cpp
    Entity/JID.cpp
    Entity/JID.h
    ini/ConfigLoader.cpp
    ini/ConfigLoader.h
    lazy/lazyq.h
    lazy/lazyq.cpp
    lazy/lazyqImpl.h
    lazy/lazyqhelper.h
    lazy/lazyqhelper.cpp
    ui/uicom.h
    ui/uicom.cpp
    ui/StyleDefine.h
    ui/StyleDefine.cpp
    ui/qimage/qimage.h
    ui/qimage/qimage.cpp
    ui/qconfig/qconfig.h
    ui/qconfig/qconfig.cpp)

set(LIBRARY_OUTPUT_PATH ../bin)
set(CMAKE_DEBUG_POSTFIX "d")
IF (CMAKE_BUILD_TYPE MATCHES "Debug")
    ADD_DEFINITIONS(-D_DEBUG)
endif ()

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
    Qt5::Xml)

if (WIN32)
    target_link_libraries(${PROJECT_NAME}
        libcurl
        libcrypto
        libssl
        zlibstat)
else ()
    target_link_libraries(${PROJECT_NAME}
        curl
        z
        ssl
        crypto)
endif ()
