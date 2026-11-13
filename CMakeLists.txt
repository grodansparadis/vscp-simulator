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
# Version year.month.patch from src/vscpsim-version.h (updated on every push
# to master by the "Update version" workflow)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/vscpsim-version.h" _vscpsim_version_lines
     REGEX "^#define[ \t]+VSCPSIM_VERSION_(YEAR|MONTH|PATCH)[ \t]+[0-9]+")
foreach(_part YEAR MONTH PATCH)
  string(REGEX MATCH "VSCPSIM_VERSION_${_part}[ \t]+([0-9]+)" _match "${_vscpsim_version_lines}")
  set(VSCPSIM_VERSION_${_part} "${CMAKE_MATCH_1}")
endforeach()
project(vscpsim
  VERSION ${VSCPSIM_VERSION_YEAR}.${VSCPSIM_VERSION_MONTH}.${VSCPSIM_VERSION_PATCH}
  LANGUAGES CXX C)
message(STATUS "VSCP simulator version ${PROJECT_VERSION}")

include(FetchContent)

cmake_policy(PUSH)
if(POLICY CMP0169)
  cmake_policy(SET CMP0169 OLD)
endif()

FetchContent_Declare(
  vscp
  GIT_REPOSITORY https://github.com/grodansparadis/vscp.git
  GIT_TAG e050031d01ffdf59a8fbeb12a27950a8003bee21
)
FetchContent_Declare(
  vscp_firmware
  GIT_REPOSITORY https://github.com/grodansparadis/vscp-firmware.git
  GIT_TAG 62ea7649f7710b0a679aa45da7bedcf89fa0453f
)
FetchContent_Declare(
  json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG 904592d2a86bae5d9c191cdcb8f8fd9b136f71fa
)
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog.git
  GIT_TAG fc7e9c8721b95661cc40e2d1d2329677d2a21387
)
FetchContent_Declare(
  mustache
  GIT_REPOSITORY https://github.com/kainjow/Mustache.git
  GIT_TAG 3f654942a70c46a775070d7a09ca7acfa3e205b7
)
FetchContent_Declare(
  maddy
  GIT_REPOSITORY https://github.com/progsource/maddy.git
  GIT_TAG 4dcdbb50be10a6262072340abcfa97ee3b4edcfa
)

foreach(dependency IN ITEMS vscp vscp_firmware json spdlog mustache maddy)
  FetchContent_GetProperties(${dependency})
  if(NOT ${dependency}_POPULATED)
    FetchContent_Populate(${dependency})
  endif()
endforeach()
cmake_policy(POP)

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
    ${vscp_SOURCE_DIR}/src/vscp/common/version.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscpremotetcpif.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscpremotetcpif.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscpdatetime.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscpdatetime.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/guid.h
    ${vscp_SOURCE_DIR}/src/vscp/common/guid.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/register.h
    ${vscp_SOURCE_DIR}/src/vscp/common/register.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/mdf.h
    ${vscp_SOURCE_DIR}/src/vscp/common/mdf.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscphelper.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscphelper.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscpunit.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscpunit.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/canaldlldef.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscpcanaldeviceif.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscpcanaldeviceif.cpp
    ${vscp_SOURCE_DIR}/src/common/vscphelper-common.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscphelperlogging.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscphelperlogging.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscphelperlogging-forward.cpp
    ${vscp_SOURCE_DIR}/src/common/vscpbase64.h
    ${vscp_SOURCE_DIR}/src/common/vscpbase64.c
    ${vscp_SOURCE_DIR}/src/common/vscp-aes.h
    ${vscp_SOURCE_DIR}/src/common/vscp-aes.c
    ${vscp_SOURCE_DIR}/src/common/crc.h
    ${vscp_SOURCE_DIR}/src/common/crc.c
    ${vscp_SOURCE_DIR}/src/common/crc8.h
    ${vscp_SOURCE_DIR}/src/common/crc8.c
    ${vscp_SOURCE_DIR}/src/common/vscpmd5.h
    ${vscp_SOURCE_DIR}/src/common/vscpmd5.c
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-base.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-base.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-canal.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-canal.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-level2.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-level2.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-mqtt.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-mqtt.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-tcp.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-tcp.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-ws1.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-ws1.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-ws2.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-ws2.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-websocket.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-websocket.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-udp.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-udp.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-multicast.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-multicast.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-bootdevice.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-bootdevice.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-bootdevice-pic1.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-bootdevice-pic1.cpp
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-bootdevice-vscp.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-bootdevice-vscp.cpp
    ${vscp_firmware_SOURCE_DIR}/src/vscp-bootloader.h
    ${vscp_firmware_SOURCE_DIR}/src/vscp-bootloader.c
    ${vscp_firmware_SOURCE_DIR}/src/vscp-firmware-helper.h
    ${vscp_firmware_SOURCE_DIR}/src/vscp-firmware-helper.c
    ${vscp_firmware_SOURCE_DIR}/src/vscp-firmware-level2.h
    ${vscp_firmware_SOURCE_DIR}/src/vscp-firmware-level2.c
    src/vscpsim-version.h
)

# SocketCAN only exists on Linux
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_sources(vscpsim PRIVATE
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-socketcan.h
    ${vscp_SOURCE_DIR}/src/vscp/common/vscp-client-socketcan.cpp
  )
  target_compile_definitions(vscpsim PRIVATE VSCPSIM_HAVE_SOCKETCAN)
endif()

# Logging in the VSCP helper library goes through spdlog
target_compile_definitions(vscpsim PRIVATE
  VSCP_ENABLE_HELPER_LOGGING
  VSCP_HELPER_LOGGING_EXPORTS
  VSCP_HELPER_LOGGING_BACKEND_EXPORTS
)

include_directories( PUBLIC ./src 
  ./build
  ./ui
  ${vscp_SOURCE_DIR}/src/vscp/common/
  ${vscp_SOURCE_DIR}/src/common
  ${vscp_firmware_SOURCE_DIR}/src
  ${json_SOURCE_DIR}/include/
  ${spdlog_SOURCE_DIR}/include/
  ${mustache_SOURCE_DIR}/
  ${maddy_SOURCE_DIR}/include/
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

