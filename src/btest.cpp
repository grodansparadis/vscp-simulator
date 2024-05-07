// btest.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB
// <info@grodansparadis.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#ifdef WIN32
#include <pch.h>
#endif

#ifndef WIN32

#else
#include <windows.h>
#endif

#include "btest.h"
#include <vscp.h>
#include <vscphelper.h>

#include "vscp_client_socketcan.h"

#include <mustache.hpp>

#include <QDebug>
#include <QDir>
#include <QFile>
// #include <QJSEngine>
#include <QJsonArray>
#include <QJsonObject>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
// #include <QSqlDatabase>
// #include <QSqlQuery>
#include <QStandardPaths>
#include <QTextDocument>
#include <QUuid>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <nlohmann/json.hpp>

// for convenience
using json = nlohmann::json;
using namespace kainjow::mustache;

///////////////////////////////////////////////////////////////////////////////
// btest
//

btest::btest(int& argc, char** argv)
  : QApplication(argc, argv)
{

  // Logging defaults
  m_fileLogLevel   = spdlog::level::info;
  m_fileLogPattern = "[%^%l%$] %v";
#ifdef WIN32
  m_fileLogPath = "btest.log";
#else
  m_fileLogPath = "~/.local/share/VSCP/btest/logs/btest.log";
#endif
  m_maxFileLogSize  = 5242880;
  m_maxFileLogFiles = 7;

  m_bEnableConsoleLog = true;
  m_consoleLogLevel   = spdlog::level::info;
  m_consoleLogPattern = "[btest] [%^%l%$] %v";

  pClient = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// ~btest
//

btest::~btest() {}

///////////////////////////////////////////////////////////////////////////////
// loadSettings *
//

void
btest::loadSettings(void)
{
  QString str;

  QSettings settings(QCoreApplication::organizationName(),
                     QCoreApplication::applicationName());

  // Configuration folder
  // --------------------
  // Linux: "/home/akhe/.config"                      Config file is here
  // (VSCP/vscp-works-qt) Windows:
  {
    QString path =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    path += "/";
    path += QCoreApplication::applicationName();
    path += "/";
    m_configFolder = settings.value("configFolder", path).toString();
  }

  // Share folder
  // ------------
  // Linux: "/home/akhe/.local/share/vscp-works-qt"   user data is here
  // Windows:
  {
    QString path =
      QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    path += "/";
    m_shareFolder = settings.value("shareFolder", path).toString();
    // If folder does not exist, create it
    QDir dir(path);
    if (!dir.exists()) {
      dir.mkpath(".");
    }
  }

// VSCP Home folder
// ----------------
#ifdef WIN32
  m_vscpHomeFolder =
    settings.value("vscpHomeFolder", "c:/program files/vscp").toString();
#else
  m_vscpHomeFolder =
    settings.value("vscpHomeFolder", "/var/lib/vscp").toString();
#endif

  // * * * Logging * * *
  m_bEnableFileLog = true;
  int level =
    settings.value("fileLogLevel", 4).toInt(); // Default: 4 == "information";
  switch (level) {
    case 0:
      m_fileLogLevel = spdlog::level::trace;
      break;
    case 1:
      m_fileLogLevel = spdlog::level::debug;
      break;
    default:
    case 2:
      m_fileLogLevel = spdlog::level::info;
      break;
    case 3:
      m_fileLogLevel = spdlog::level::warn;
      break;
    case 4:
      m_fileLogLevel = spdlog::level::err;
      break;
    case 5:
      m_fileLogLevel = spdlog::level::critical;
      break;
    case 6:
      m_fileLogLevel   = spdlog::level::off;
      m_bEnableFileLog = false;
      break;
  };
  m_fileLogPattern = settings.value("fileLogPattern", "%c - [%^%l%$] %v")
                       .toString()
                       .toStdString();
  m_fileLogPath =
    settings.value("fileLogPath", "~/.local/share/VSCP/btest/logs/btest.log")
      .toString()
      .toStdString();
  m_maxFileLogSize  = settings.value("fileLogMaxSize", 5 * 1024 * 1024).toInt();
  m_maxFileLogFiles = settings.value("fileLogMaxFiles", 10).toInt();

  // console log level
  m_bEnableConsoleLog = true;
  level               = settings.value("consoleLogLevel", 4)
            .toInt(); // Default: 4 == "information";
  switch (level) {
    case 0:
      m_consoleLogLevel = spdlog::level::trace;
      break;
    case 1:
      m_consoleLogLevel = spdlog::level::debug;
      break;
    default:
    case 2:
      m_consoleLogLevel = spdlog::level::info;
      break;
    case 3:
      m_consoleLogLevel = spdlog::level::warn;
      break;
    case 4:
      m_consoleLogLevel = spdlog::level::err;
      break;
    case 5:
      m_consoleLogLevel = spdlog::level::critical;
      break;
    case 6:
      m_consoleLogLevel   = spdlog::level::off;
      m_bEnableConsoleLog = false;
      break;
  };

  m_consoleLogPattern = settings.value("consoleLogPattern", "%c [%^%l%$] %v")
                          .toString()
                          .toStdString();

  settings.endArray();
}

///////////////////////////////////////////////////////////////////////////////
// writeSettings
//

void
btest::writeSettings()
{
  QSettings settings(QCoreApplication::organizationName(),
                     QCoreApplication::applicationName());

  // General settings
  // settings.setValue("configFolder", m_configFolder);

  // * * * Logging * * *
  int level = 4; // Default: 4 == "information";
  switch (m_fileLogLevel) {
    case spdlog::level::trace:
      level = 0;
      break;
    case spdlog::level::debug:
      level = 1;
      break;
    default:
    case spdlog::level::info:
      level = 2;
      break;
    case spdlog::level::warn:
      level = 3;
      break;
    case spdlog::level::err:
      level = 4;
      break;
    case spdlog::level::critical:
      level = 5;
      break;
    case spdlog::level::off:
      level = 6;
      break;
  };

  settings.setValue("fileLogLevel", level);
  settings.setValue("fileLogPattern", QString::fromStdString(m_fileLogPattern));
  settings.setValue("fileLogPath", QString::fromStdString(m_fileLogPath));
  settings.setValue("fileLogMaxSize", m_maxFileLogSize);
  settings.setValue("fileLogMaxFiles", m_maxFileLogFiles);

  level = 4; // Default: 4 == "information";
  switch (m_consoleLogLevel) {
    case spdlog::level::trace:
      level = 0;
      break;
    case spdlog::level::debug:
      level = 1;
      break;
    default:
    case spdlog::level::info:
      level = 2;
      break;
    case spdlog::level::warn:
      level = 3;
      break;
    case spdlog::level::err:
      level = 4;
      break;
    case spdlog::level::critical:
      level = 5;
      break;
    case spdlog::level::off:
      level = 6;
      break;
  };

  settings.setValue("consoleLogLevel", level);
  settings.setValue("consoleLogPattern",
                    QString::fromStdString(m_consoleLogPattern));
}

///////////////////////////////////////////////////////////////////////////////
// getMainWindow
//

QMainWindow*
btest::getMainWindow()
{
  foreach (QWidget* w, qApp->topLevelWidgets()) {
    if (QMainWindow* mainWin = qobject_cast<QMainWindow*>(w)) {
      return mainWin;
    }
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
//                  Handlers for VSCP Bootloader callbacks
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// goApplication
//
// This code jumps to the application. Typically this is code
//  like
//
//  void (*foo)(void) = (void (*)())0x12345678;
//  foo();
//
//  where 0x12345678 is the address to jump to.
//

void
btest::vscpboot_goApplication(void)
{
  btest* pbtest = (btest*)QApplication::instance();
}

/*!
  Initialize hardware. This should be the first method called.
  The communication interface should be opended here and the system
  should be ready to receive firmware code events when done.
*/
int
btest::vscpboot_init_hardware(void)
{
  int rv;

  if (m_interface == "socketcan") {

    if (!m_configVector.size()) {
      spdlog::error("Need socketcan interface but it is not given {0}", m_interface.toStdString());
      return VSCP_ERROR_PARAMETER;
    }

    pClient = new vscpClientSocketCan();
    if (nullptr == pClient) {
      spdlog::error("Unable to create client object {0}", m_interface.toStdString());
      return VSCP_ERROR_ERROR;
    }

    if (VSCP_ERROR_SUCCESS !=
        (rv = ((vscpClientSocketCan*)pClient)->init(m_configVector[0], m_guid.toString(), 0))) {
      delete (vscpClientSocketCan*)pClient;
      pClient = nullptr;
      spdlog::error("Unable to initialize socketcan client {0} rv={1}", m_interface.toStdString(), rv);
      return VSCP_ERROR_HARDWARE;
    }

    if (VSCP_ERROR_SUCCESS != (rv = pClient->connect())) {
      delete (vscpClientSocketCan*)pClient;
      pClient = nullptr;
      spdlog::error("Unable to initialize socketcan client {0} rv={1}", m_interface.toStdString(), rv);
      return VSCP_ERROR_HARDWARE;
    }
  }
  else {
    spdlog::error("Client type is not supported {0}", m_interface.toStdString());
    return VSCP_ERROR_NOT_SUPPORTED;
  }

  return VSCP_ERROR_SUCCESS;
}

/*!
  Free any hardware resources that neds to be free'd
*/
int
btest::vscpboot_release_hardware()
{
  int rv;
  if (m_interface == "socketcan") {

    if (VSCP_ERROR_SUCCESS != (rv = pClient->disconnect())) {
      delete (vscpClientSocketCan*)pClient;
      pClient = nullptr;
      spdlog::error("Unable to initialize socketcan client {0} rv={1}", m_interface.toStdString(), rv);
      return VSCP_ERROR_HARDWARE;
    }


    delete (vscpClientSocketCan*)pClient;
    pClient = nullptr;
  }
  return VSCP_ERROR_SUCCESS;
}

/*!
  Get Device bootflag from persistent storage.
  @return Bootflag for node. Zero means application should start. Nonzero
    means bootloader should start. Application can use other values for
    bootflag or multibyte storage as long as this routine report expected state.
*/
uint8_t
btest::vscpboot_getBootFlag(void)
{
  return 0;
}

/*!
  Set Device bootflag in persistent storage. Application can use other values
  for bootflag or multibyte storage as long as this routine set proper state.
  @param bootflag. New bootflag to set. Zero means application should start.
  Nonzero means bootloader should start.
*/
int
btest::vscpboot_setBootFlag(uint8_t bootflag)
{
  return VSCP_ERROR_SUCCESS;
}

/*!
  Reboot the board. This method should never return.
*/
void
btest::vscpboot_reboot(void)
{
}

/*!
  Get GUID for device.
  On some devices GUID may be programmed by the firmware load
  process. This is typical for devices that have a id/userid
  in flash. For devices using Ethernet etc construct GUID in
  the usual manner.
  @return Pointer to GUID
*/
uint8_t*
btest::vscpboot_getGUID(void)
{
  return nullptr;
}

/*!
  Check if memtype is valid. type == 0 is always valid
  @return non zero if valid
*/
int
btest::vscpboot_isMemTypeValid(uint8_t type)
{
  return FALSE;
}

/*!
  Check if membank is valid. type == 0 is always valid
  @return non zero if valid
*/
int
btest::vscpboot_isMemBankValid(uint8_t bank)
{
  return FALSE;
}

/*!
  Program a block
  @param pblock Pointer to the block to program
  @param type Memory type
  @param bank Memory bank to program
  @return VSCP_ERROR_SUCCESS on success and else errocode
*/
int
btest::vscpboot_programBlock(const uint8_t* pblock, uint8_t type, uint8_t bank)
{
  return VSCP_ERROR_SUCCESS;
}

/*!
  The CRC for the loaded data is calculated here. This is the CRC
  over all blocks programmed calulated with CRC-CCITT. For a successful
  programming this value should be the same as the one provided in the
  activate new image event.
  @return crc CRC-CCITT for programmed area.
*/
crc
btest::vscpboot_calcPrgCrc(void)
{
  return 0;
}

/*!
  Send VSCP event
  @param pex Pointer to VSCP event ex.
  @return VSCP_ERROR_SUCCESS on success
*/
int
btest::vscpboot_sendEvent(vscpEventEx* pex)
{
  return pClient->send(*pex);
}

/*!
  Get VSCP event
  -----------------------------------------------------------
  IMPORTANT!
  This routine should translate all VSCP_CLASS2_LEVEL1_PROTOCOL
  events to VSCP_CLASS1_PROTOCOL events.
  -----------------------------------------------------------
  @param Pointer to VSCP event structure.
   @return VSCP_ERROR_SUCCESS on success
*/
int
btest::vscpboot_getEvent(vscpEventEx* pex)
{
  return VSCP_ERROR_SUCCESS;
}