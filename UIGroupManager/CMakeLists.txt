project(UIGroupManager)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt5Widgets)
find_package(Qt5Concurrent)

#
set(SOURCE_FILES
        UIGroupManager.cpp
        UIGroupManager.h
        uigroupmanager_global.h
        UIGroupManagerPlug.cpp
        UIGroupManagerPlug.h
        SearchWgt.cpp
        SearchWgt.h
        MessageManager.cpp
        MessageManager.h
        ChoseMemberWgt.cpp
        ChoseMemberWgt.h
        TreeItemDelegate.cpp
        TreeItemDelegate.h
        image1.qrc
        BatchAddMemberWnd.cpp
        BatchAddMemberWnd.h
        AddGroupEdit.cpp
        AddGroupEdit.h SearchView.cpp SearchView.h)

if(APPLE)
    ADD_DEFINITIONS(-D_MACOS)
elseif(UNIX)
    ADD_DEFINITIONS(-D_LINUX)
else()
    ADD_DEFINITIONS(-D_WINDOWS)
    ADD_DEFINITIONS(-DUIGROUPMANAGER_LIB)
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
        Qt5::Concurrent
        CustomUi
        EventBus
        DataCenter
        Util)
