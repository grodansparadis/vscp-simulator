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

#ifndef BTEST_H
#define BTEST_H

#include <vscp.h>

#include <guid.h>
#include <mdf.h>
#include <register.h>
#include <version.h>
#include <vscp_client_base.h>
#include <vscpunit.h>

#include <QApplication>
#include <QByteArray>
#include <QDateTime>
#include <QMainWindow>
#include <QMutex>
#include <QObject>

#include <list>

#include <mustache.hpp>
#include <nlohmann/json.hpp>

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

// https://github.com/nlohmann/json
using json = nlohmann::json;

using namespace kainjow::mustache;

// Register to be used for signals
Q_DECLARE_METATYPE(vscpEvent)
Q_DECLARE_METATYPE(vscpEventEx)

/// Application name used in titles and headers
#define APPNAME "Boot test"

// home folder is used for storage of program configuration
// system folder holds databases etc
#ifdef WIN32
#define DEFAULT_HOME_FOLDER        "c:/programdata/vscp/btest/"
#define DEFAULT_VSCP_SYSTEM_FOLDER "c:/programdata/vscp/"
#else
#define DEFAULT_HOME_FOLDER        "~/.btest/"
#define DEFAULT_VSCP_SYSTEM_FOLDER "/var/lib/vscp/"
#endif

/*!
    Encapsulates VSCP works main settings
*/
class btest : public QApplication {

  Q_OBJECT

public:
  /*!
      btest
  */
  btest(int& argc, char** argv);

  /*!
      Destructor
  */
  ~btest();

  /*!
    Codes for mode we are in
  */
  enum mode {
    BOOT_MODE,
    FIRMWARE_MODE
  };

  /*!
      Load configuration settings from disk
  */
  void loadSettings(void);

  /*!
      Save configuration settings to disk
  */
  void writeSettings(void);

  QMainWindow* getMainWindow();

  // ========================================================================
  //                         Master callbacks
  // ========================================================================

  void vscpboot_goApplication(void);

  int vscpboot_init_hardware(void);

  int vscpboot_release_hardware(void);

  uint8_t vscpboot_getBootFlag(void);

  int vscpboot_setBootFlag(uint8_t bootflag);

  void vscpboot_reboot(void);

  uint8_t* vscpboot_getGUID(void);

  int vscpboot_isMemTypeValid(uint8_t type);

  int vscpboot_isMemBankValid(uint8_t bank);

  int vscpboot_programBlock(const uint8_t* pblock, uint8_t type, uint8_t bank);

  crc vscpboot_calcPrgCrc(void);

  int vscpboot_sendEvent(vscpEventEx* pex);

  int vscpboot_getEvent(vscpEventEx* pex);

  // ========================================================================
  // ========================================================================

  // ------------------------------------------------------------------------
  // Global Configuration information below
  //   This info is read from a configuration file
  //   at startup and saved on close. The configuration
  //   file should be placed in the home folder.
  // ------------------------------------------------------------------------

  // ------------------------------------------------------------------------

  /// Folder used for configuration
  /// Linux: ~/.configure/VSCP/(btest.conf)
  QString m_configFolder;

  /// Folder for writeable data
  /// Linux: ~/.local/share/vscp/btest
  QString m_shareFolder;

  // Folder used for VSCP files like db's
  // Linux:
  // vscp/drivers/level1 - contain level one drivers
  // vscp/drivers/level2 - contain level two drivers
  // Windows:
  // c:/program data/vscp/drivers/level1
  // c:/program data/vscp/drivers/level2
  QString m_vscpHomeFolder;

  /*!
    Thhe communication interface
  */
  CVscpClient* pClient;

  // ---------------------------------------------------

  /// Interface
  QString m_interface;

  /// Configuration String
  QString m_config;

  std::deque<std::string> m_configVector;

  /// GUID
  cguid m_guid;

  /// Host
  QString m_host;

  /// Port
  uint16_t m_port;

  /// User
  QString m_user;

  /// Password
  QString m_password;

  //**************************************************************************
  //                            LOGGER (SPDLOG)
  //**************************************************************************

  bool m_bEnableFileLog;
  spdlog::level::level_enum m_fileLogLevel;
  std::string m_fileLogPattern;
  std::string m_fileLogPath;
  uint32_t m_maxFileLogSize;
  uint16_t m_maxFileLogFiles;

  bool m_bEnableConsoleLog;
  spdlog::level::level_enum m_consoleLogLevel;
  std::string m_consoleLogPattern;
};

#endif
