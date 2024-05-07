cmake_minimum_required(VERSION 3.16)

project(btest VERSION 1.0.0 LANGUAGES CXX C)

## --- C++14 build flags ---
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

if (MSVC)
    # warning level 4
    add_compile_options(/W4)
else()
    # additional warnings
    #add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# --- Path to vscp --- 
if (NOT DEFINED VSCP_PATH)
  set (VSCP_PATH "${PROJECT_SOURCE_DIR}/../vscp")
endif()

file(TO_CMAKE_PATH "$ENV{VSCP_ROOT}" VSCP_PATH)
message(STATUS "VSCP path = $ENV{VSCP_ROOT}")

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
  $ENV{VSCP_ROOT}/src/vscp/common/  
  $ENV{VSCP_ROOT}/src/common
  $ENV{VSCP_FIRMWARE}/common
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
    src/btest.cpp
    src/hardware.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/version.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscpremotetcpif.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscpremotetcpif.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/vscpdatetime.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscpdatetime.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/guid.h
    $ENV{VSCP_ROOT}/src/vscp/common/guid.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/register.h
    $ENV{VSCP_ROOT}/src/vscp/common/register.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/mdf.h
    $ENV{VSCP_ROOT}/src/vscp/common/mdf.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/vscphelper.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscphelper.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/vscpunit.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscpunit.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/canaldlldef.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscpcanaldeviceif.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscpcanaldeviceif.cpp
    $ENV{VSCP_ROOT}/src/common/sockettcp.h
    $ENV{VSCP_ROOT}/src/common/sockettcp.c
    $ENV{VSCP_ROOT}/src/common/vscpbase64.h
    $ENV{VSCP_ROOT}/src/common/vscpbase64.c
    $ENV{VSCP_ROOT}/src/common/vscp_aes.h
    $ENV{VSCP_ROOT}/src/common/vscp_aes.c
    $ENV{VSCP_ROOT}/src/common/crc.h
    $ENV{VSCP_ROOT}/src/common/crc.c
    $ENV{VSCP_ROOT}/src/common/crc8.h
    $ENV{VSCP_ROOT}/src/common/crc8.c
    $ENV{VSCP_ROOT}/src/common/vscpmd5.h
    $ENV{VSCP_ROOT}/src/common/vscpmd5.c
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_base.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_base.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_canal.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_canal.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_mqtt.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_mqtt.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_tcp.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_tcp.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_socketcan.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_socketcan.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_ws1.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_ws1.cpp 
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_ws2.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_ws2.cpp 
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_udp.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_udp.cpp 
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_multicast.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_multicast.cpp 
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_rawcan.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_rawcan.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_rawmqtt.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_client_rawmqtt.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_bootdevice.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_bootdevice.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_bootdevice_pic1.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_bootdevice_pic1.cpp
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_bootdevice_vscp.h
    $ENV{VSCP_ROOT}/src/vscp/common/vscp_bootdevice_vscp.cpp
    $ENV{VSCP_FIRMWARE}/common/vscp-bootloader.h
    $ENV{VSCP_FIRMWARE}/common/vscp-bootloader.c
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
