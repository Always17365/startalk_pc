project(UIOAManager)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Widgets)
find_package(Qt5WebEngineWidgets)
find_package(Qt5Concurrent)

#
set(SOURCE_FILES
        image1.qrc
        NavigationItem.cpp
        NavigationItem.h
        oamanager.json
        OAManagerPanel.cpp
        OAManagerPanel.h
        OAManagerPlug.cpp
        OAManagerPlug.h
        uioamanager_global.h
        OaMainWgt.cpp
        OaMainWgt.h
        OAMainWidget.cpp
        OAMainWidget.h
        OADefine.h
        MessageManager.cpp
        MessageManager.h)

if(APPLE)
    ADD_DEFINITIONS(-D_MACOS)
elseif(UNIX)
    ADD_DEFINITIONS(-D_LINUX)
else()
    ADD_DEFINITIONS(-D_WINDOWS)
    ADD_DEFINITIONS(-DUIOAMANAGER_LIB)

    link_directories(../Depends/curl/lib/windows/x64)
    link_directories(../Depends/zlib/lib/windows/x64)
    link_directories(../Depends/openssl/lib/windows/x64)
endif()

set(LIBRARY_OUTPUT_PATH  ../bin)
set (CMAKE_DEBUG_POSTFIX "d")


ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCE_FILES})

#添加链接库
target_link_libraries(${PROJECT_NAME}
        Qt5::Widgets
        Qt5::WebEngineWidgets
        Qt5::Concurrent
        EventBus
        DataCenter
        WebService)
