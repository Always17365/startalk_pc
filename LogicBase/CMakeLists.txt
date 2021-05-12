project(LogicBase)

include_directories(.)

if (APPLE)
    ADD_DEFINITIONS(-D_MACOS)
elseif (UNIX)
    ADD_DEFINITIONS(-D_LINUX)
else ()
    ADD_DEFINITIONS(-D_WINDOWS)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
    link_libraries(Ws2_32.lib)
    link_libraries(Wldap32.lib)
    link_libraries(Crypt32.lib)
endif ()

#
set(SOURCE_FILES
        dllMain.cpp
        logicbase_global.h
        Protobuf/message.pb.cc
        Protobuf/message.pb.h
        nativeSocket.cpp
        ParseFailedException.h
        ProtobufParser.cpp
        ProtobufParser.h
        ProtobufStack.cpp
        ProtobufStack.h
        ProtobufStream.cpp
        ProtobufStream.h
        Stream.h
        QTalkSecurity.cpp
        QTalkSecurity.h
        RSAHelper.cpp
        RSAHelper.h
        LogicBase.cpp
        LogicBase.h
        MessageManager.cpp
        MessageManager.h
        BaseUtil.hpp
        ProtobufStreamDelegate.h
        BaseUtil.cpp
        pb2json.cpp pb2json.h
        MemoryStream.h)


set(LIBRARY_OUTPUT_PATH ../bin)
set(CMAKE_DEBUG_POSTFIX "d")
IF (CMAKE_BUILD_TYPE MATCHES "Debug")
    ADD_DEFINITIONS(-DDEBUG)
endif ()

ADD_LIBRARY(${PROJECT_NAME} SHARED ${SOURCE_FILES})

#添加链接库
if (WIN32)
    target_link_libraries(${PROJECT_NAME}
            libssl
            libcrypto
            libprotobuf
            LogicManager
            DataBasePlug
            QtUtil
            Platform
            EventBus)
else ()
    target_link_libraries(${PROJECT_NAME}
            ssl
            crypto
            protobuf
            LogicManager
            DataBasePlug
            QtUtil
            Platform
            EventBus)
endif ()
