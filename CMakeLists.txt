# CMakeLists.txt
#
# Build instructions for VSCP Simulator
#
# Copyright (c) 2000-2025 Åke Hedman, Grodans Paradis AB
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# VSCP_PATH - set to path to local vscp repository. Default is "../vscp". Set with
# -DVSCP_PATH="path"
#
# windows
# -------
# cmake .. -G "Visual Studio 17 2022" -DVCPKG_TARGET_TRIPLET=x64-windows  -DCMAKE_TOOLCHAIN_FILE=G:\vcpkg\scripts\buildsystems\vcpkg.cmake -DCMAKE_PREFIX_PATH=G:\Qt\6.7.2\msvc2019_64\
# cmake --build . --config Debug
# set PATH=G:\Qt\Qt-6.7.2\msvc2019_64\bin;%PATH%
#

cmake_minimum_required(VERSION 3.16)
project(vscpsim VERSION 1.0.0 LANGUAGES CXX C)

## --- C++14 build flags ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set_target_properties(${PROJECT_NAM} PROPERTIES
  WIN32_EXECUTABLE ON
  MACOSX_BUNDLE ON
)

# --- System Libraries ---
include(GNUInstallDirs) 

# --- Path to vscp --- 
if (NOT DEFINED VSCP_PATH)
  set (VSCP_PATH "${PROJECT_SOURCE_DIR}/../vscp")
endif()

file(TO_CMAKE_PATH "$ENV{VSCP_ROOT}" VSCP_PATH)
message(STATUS "VSCP path = $ENV{VSCP_ROOT}")

file(TO_CMAKE_PATH "$ENV{VSCP_FIRMWARE}" VSCP_FIRMWARE)
message(STATUS "VSCP firmware path = $ENV{VSCP_FIRMWARE}")
message(STATUS "VSCP firmware path = ${VSCP_FIRMWARE}")

file(TO_CMAKE_PATH "$ENV{QT_ROOT_DIR}" QT_PATH)
message(STATUS "QT_ROOT_DIR = $ENV{QT_ROOT_DIR}")

if (MSVC)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
  add_definitions(-DWIN32_LEAN_AND_MEANS)
  add_definitions(-D_POSIX_C_SOURCE)    
  add_definitions(-DVCPKG_TARGET_TRIPLET=x64-windows)
  add_definitions(-D_SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
  add_definitions(-D_SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS)
  set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  set(LIBS_SYSTEM ws2_32)
  # use precompiled headers
  set (CMAKE_AUTOMOC_MOC_OPTIONS "-bpch.h ")
  # warning level 4
  add_compile_options(/W4)
else()
  set(LIBS_SYSTEM c stdc++)
  # additional warnings
  #add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# set the module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

find_package(Qt6 REQUIRED COMPONENTS Widgets Qml)
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



qt_add_executable(vscpsim
    src/mainwindow.ui
    src/mainwindow.cpp
    src/main.cpp
    src/btest.cpp
    src/hardware.cpp
    third_party/vscp/src/vscp/common/version.h
    third_party/vscp/src/vscp/common/vscp.h
    third_party/vscp/src/vscp/common/vscpremotetcpif.h
    third_party/vscp/src/vscp/common/vscpremotetcpif.cpp
    third_party/vscp/src/vscp/common/vscpdatetime.h
    third_party/vscp/src/vscp/common/vscpdatetime.cpp
    third_party/vscp/src/vscp/common/guid.h
    third_party/vscp/src/vscp/common/guid.cpp
    third_party/vscp/src/vscp/common/register.h
    third_party/vscp/src/vscp/common/register.cpp
    third_party/vscp/src/vscp/common/mdf.h
    third_party/vscp/src/vscp/common/mdf.cpp
    third_party/vscp/src/vscp/common/vscphelper.h
    third_party/vscp/src/vscp/common/vscphelper.cpp
    third_party/vscp/src/vscp/common/vscpunit.h
    third_party/vscp/src/vscp/common/vscpunit.cpp
    third_party/vscp/src/vscp/common/canaldlldef.h
    third_party/vscp/src/vscp/common/vscpcanaldeviceif.h
    third_party/vscp/src/vscp/common/vscpcanaldeviceif.cpp
    third_party/vscp/src/common/sockettcp.h
    third_party/vscp/src/common/sockettcp.c
    third_party/vscp/src/common/vscpbase64.h
    third_party/vscp/src/common/vscpbase64.c
    third_party/vscp/src/common/vscp-aes.h
    third_party/vscp/src/common/vscp-aes.c
    third_party/vscp/src/common/crc.h
    third_party/vscp/src/common/crc.c
    third_party/vscp/src/common/crc8.h
    third_party/vscp/src/common/crc8.c
    third_party/vscp/src/common/vscpmd5.h
    third_party/vscp/src/common/vscpmd5.c
    third_party/vscp/src/vscp/common/vscp-client-base.h
    third_party/vscp/src/vscp/common/vscp-client-base.cpp
    third_party/vscp/src/vscp/common/vscp-client-canal.h
    third_party/vscp/src/vscp/common/vscp-client-canal.cpp
    third_party/vscp/src/vscp/common/vscp-client-mqtt.h
    third_party/vscp/src/vscp/common/vscp-client-mqtt.cpp
    third_party/vscp/src/vscp/common/vscp-client-tcp.h
    third_party/vscp/src/vscp/common/vscp-client-tcp.cpp
    third_party/vscp/src/vscp/common/vscp-client-socketcan.h
    third_party/vscp/src/vscp/common/vscp-client-socketcan.cpp
    third_party/vscp/src/vscp/common/vscp-client-ws1.h
    third_party/vscp/src/vscp/common/vscp-client-ws1.cpp 
    third_party/vscp/src/vscp/common/vscp-client-ws2.h
    third_party/vscp/src/vscp/common/vscp-client-ws2.cpp 
    third_party/vscp/src/vscp/common/vscp-client-udp.h
    third_party/vscp/src/vscp/common/vscp-client-udp.cpp 
    third_party/vscp/src/vscp/common/vscp-client-multicast.h
    third_party/vscp/src/vscp/common/vscp-client-multicast.cpp 
    third_party/vscp/src/vscp/common/vscp-bootdevice.h
    third_party/vscp/src/vscp/common/vscp-bootdevice.cpp
    third_party/vscp/src/vscp/common/vscp-bootdevice-pic1.h
    third_party/vscp/src/vscp/common/vscp-bootdevice-pic1.cpp
    third_party/vscp/src/vscp/common/vscp-bootdevice-vscp.h
    third_party/vscp/src/vscp/common/vscp-bootdevice-vscp.cpp
    third_party/vscp-firmware/common/vscp-bootloader.h
    third_party/vscp-firmware/common/vscp-bootloader.c
    third_party/vscp-firmware/common/vscp-firmware-helper.h
    third_party/vscp-firmware/common/vscp-firmware-helper.c
    third_party/vscp-firmware/common/vscp-firmware-level2.h
    third_party/vscp-firmware/common/vscp-firmware-level2.c
)

include_directories( PUBLIC ./src 
  ./build
  ./ui
  third_party/vscp/src/vscp/common/
  third_party/vscp/src/common
  third_party/vscp-firmware/common
  ./third_party
  ./third_party/json/include/
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

target_link_libraries(vscpsim PRIVATE 
  Qt6::Widgets
  Qt6::Qml
  ${CMAKE_DL_LIBS}
  Threads::Threads
  OpenSSL::SSL 
  OpenSSL::Crypto
  ${OPENSSL_LIBRARIES}
  ${MOSQUITTO_LIBRARIES}
  ${LIBWEBSOCKETS_LIBRARIES}
  ${EXPAT_LIBRARIES}
  ${CURL_LIBRARIES}
)

install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${CMAKE_SOURCE_DIR}/install/${PROJECT_NAME}.desktop
  DESTINATION ${INSTALL_PATH}/share/applications/) 

# icons
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/icons/ DESTINATION ${INSTALL_PATH}/share/icons/) 

# Event database etc
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/share/ DESTINATION ${INSTALL_PATH}/share/${PROJECT_NAME}/)

