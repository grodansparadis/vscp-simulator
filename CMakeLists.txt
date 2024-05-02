cmake_minimum_required(VERSION 3.16)

project(btest VERSION 1.0.0 LANGUAGES CXX C)

## --- C++14 build flags ---
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# --- Path to vscp --- 
if (NOT DEFINED VSCP_PATH)
  set (VSCP_PATH "${PROJECT_SOURCE_DIR}/../vscp")
endif()

file(TO_CMAKE_PATH "${VSCP_PATH}" VSCP_PATH)
message(STATUS "VSCP path = ${VSCP_PATH}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# set the module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

# pthreads
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# openssl
find_package(OpenSSL REQUIRED)

# libwebsockets
#find_package(libwebsockets CONFIG REQUIRED)
include(FindLibWebSockets)

# expat
include(FindEXPAT)
find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})

# Mosquitto lib
include(FindMosquitto)

find_package(CURL REQUIRED) 
IF(CURL_FOUND)
  message(STATUS Found cURL)
  INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIR})
  SET(requiredlibs ${requiredlibs} ${CURL_LIBRARIES} )
ELSE(CURL_FOUND)
  MESSAGE(FATAL_ERROR "Could not find the CURL library.")
ENDIF(CURL_FOUND)

set(OPENSSL_VER "OPENSSL_API_${OPENSSL_VERSION_MAJOR}_${OPENSSL_VERSION_MINOR}")
message(STATUS "OPENSSL Version: ${OPENSSL_VER} - ${OPENSSL_VERSION}")
#add_definitions(-D${OPENSSL_VER}) 

# 1.1.0 / 1.1.1 / 3.0.0 / 3.1.0 / 3.2.0
string(FIND "${OPENSSL_VERSION}" "1.0" SSL_VER_POS)
if(NOT ${SSL_VER_POS} EQUAL -1)
  add_definitions(-DOPENSSL_API_1_0)
  message(STATUS "Open ssl version is 1.0")
endif()    

string(FIND "${OPENSSL_VERSION}" "1.1" SSL_VER_POS)
if(NOT ${SSL_VER_POS} EQUAL -1)
  add_definitions(-DOPENSSL_API_1_1)
  message(STATUS "Open ssl version is 1.1")
endif()

string(FIND "${OPENSSL_VERSION}" "3.0" SSL_VER_POS)
if(NOT SSL_VER_POS EQUAL -1)
  add_definitions(-DOPENSSL_API_3_0)
  message(STATUS "Open ssl version is 3.0")
endif()  

string(FIND "${OPENSSL_VERSION}" "3.1" SSL_VER_POS)
if(NOT SSL_VER_POS EQUAL -1)
  add_definitions(-DOPENSSL_API_3_1)
  message(STATUS "Open ssl version is 3.1")
endif()

string(FIND "${OPENSSL_VERSION}" "3.2" SSL_VER_POS)
if(NOT SSL_VER_POS EQUAL -1)
  add_definitions(-DOPENSSL_API_3_2)
  message(STATUS "Open ssl version is 3.2")
endif()

include_directories( PUBLIC ./src 
  ./build
  ./ui
  ${VSCP_PATH}/src/vscp/common/  
  ${VSCP_PATH}/src/common
  ./third_party
  ./third_party/nlohmann/include/
  ./third_party/spdlog/include/
  ./third_party/Mustache/
  ./third_party/maddy/include/
  ./third_party/fastpbkdf2/
  ${CMAKE_BINARY_DIR}
  ${OPENSSL_INCLUDE_DIR}
  ${MOSQUITTO_INCLUDE_DIRS}
  ${LIBWEBSOCKETS_INCLUDE_DIR}
  ${CURL_INCLUDE_DIR}
)

qt_add_executable(btest
    src/mainwindow.ui
    src/mainwindow.cpp
    src/main.cpp
    ${VSCP_PATH}/src/vscp/common/version.h
    ${VSCP_PATH}/src/vscp/common/vscp.h
    ${VSCP_PATH}/src/vscp/common/vscpremotetcpif.h
    ${VSCP_PATH}/src/vscp/common/vscpremotetcpif.cpp
    ${VSCP_PATH}/src/vscp/common/vscpdatetime.h
    ${VSCP_PATH}/src/vscp/common/vscpdatetime.cpp
    ${VSCP_PATH}/src/vscp/common/guid.h
    ${VSCP_PATH}/src/vscp/common/guid.cpp
    ${VSCP_PATH}/src/vscp/common/register.h
    ${VSCP_PATH}/src/vscp/common/register.cpp
    ${VSCP_PATH}/src/vscp/common/mdf.h
    ${VSCP_PATH}/src/vscp/common/mdf.cpp
    ${VSCP_PATH}/src/vscp/common/vscphelper.h
    ${VSCP_PATH}/src/vscp/common/vscphelper.cpp
    ${VSCP_PATH}/src/vscp/common/vscpunit.h
    ${VSCP_PATH}/src/vscp/common/vscpunit.cpp
    ${VSCP_PATH}/src/vscp/common/canaldlldef.h
    ${VSCP_PATH}/src/vscp/common/vscpcanaldeviceif.h
    ${VSCP_PATH}/src/vscp/common/vscpcanaldeviceif.cpp
    ${VSCP_PATH}/src/common/sockettcp.h
    ${VSCP_PATH}/src/common/sockettcp.c
    ${VSCP_PATH}/src/common/vscpbase64.h
    ${VSCP_PATH}/src/common/vscpbase64.c
    ${VSCP_PATH}/src/common/vscp_aes.h
    ${VSCP_PATH}/src/common/vscp_aes.c
    ${VSCP_PATH}/src/common/crc.h
    ${VSCP_PATH}/src/common/crc.c
    ${VSCP_PATH}/src/common/crc8.h
    ${VSCP_PATH}/src/common/crc8.c
    ${VSCP_PATH}/src/common/vscpmd5.h
    ${VSCP_PATH}/src/common/vscpmd5.c
    ${VSCP_PATH}/src/vscp/common/vscp_client_base.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_base.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_canal.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_canal.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_mqtt.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_mqtt.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_tcp.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_tcp.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_socketcan.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_socketcan.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_ws1.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_ws1.cpp 
    ${VSCP_PATH}/src/vscp/common/vscp_client_ws2.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_ws2.cpp 
    ${VSCP_PATH}/src/vscp/common/vscp_client_udp.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_udp.cpp 
    ${VSCP_PATH}/src/vscp/common/vscp_client_multicast.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_multicast.cpp 
    ${VSCP_PATH}/src/vscp/common/vscp_client_rawcan.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_rawcan.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_rawmqtt.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_rawmqtt.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_bootdevice.h
    ${VSCP_PATH}/src/vscp/common/vscp_bootdevice.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_bootdevice_pic1.h
    ${VSCP_PATH}/src/vscp/common/vscp_bootdevice_pic1.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_bootdevice_vscp.h
    ${VSCP_PATH}/src/vscp/common/vscp_bootdevice_vscp.cpp
#   ./third_party/Mustache/mustache.hpp
#   ./third_party/spdlog/include/
    ./third_party/fastpbkdf2/fastpbkdf2.h
    ./third_party/fastpbkdf2/fastpbkdf2.c
)


target_link_libraries(btest PRIVATE Qt6::Widgets
  Threads::Threads
  OpenSSL::SSL 
  OpenSSL::Crypto
  ${OPENSSL_LIBRARIES}
  ${MOSQUITTO_LIBRARIES}
  ${LIBWEBSOCKETS_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${CURL_LIBRARIES}
)

set_target_properties(btest PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
