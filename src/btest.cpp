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

#include <deque>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <stdio.h>
#include <string>

#include "btest.h"
#include <vscp.h>
#include <vscphelper.h>

#include "vscp-client-socketcan.h"

#include <vscp-bootloader.h>

#include <mustache.hpp>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJSEngine>
#include <QJsonArray>
#include <QJsonObject>
#include <QMainWindow>
#include <QMessageBox>
#include <QSemaphore>
#include <QSettings>
#include <QStandardPaths>
#include <QTextDocument>
#include <QUuid>

#include <expat.h>
#include <maddy/parser.h> // Markdown -> HTML
#include <mustache.hpp>
#include <nlohmann/json.hpp>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

// for convenience
using json = nlohmann::json;
using namespace kainjow::mustache;

// Prototype
void*
workerThread(void* pData);

///////////////////////////////////////////////////////////////////////////////
// btest
//

btest::btest(int& argc, char** argv)
  : QApplication(argc, argv)
{
  m_threadWork = 0;

  m_bootflag = BOOTLOADER; // Start the bootloader

  // Set default connect timout
  m_timeoutConnect = DEFAULT_CONNECT_TIMOUT;

  m_bootloader_cfg.vscpLevel  = VSCP_LEVEL2;
  m_bootloader_cfg.blockSize  = 0x100;
  m_bootloader_cfg.blockCount = 0xffff;

  // Config file
  m_configFolder =
    QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
  m_configFolder += "/VSCP/";
  m_configFolder += QCoreApplication::applicationName();
  m_configFolder += ".conf";

  // Logging defaults
  m_fileLogLevel   = spdlog::level::info;
  m_fileLogPattern = "[%^%l%$] %v";
#ifdef WIN32
  m_fileLogPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toStdString();
  m_fileLogPath += "/VSCP/btest/btest.log";
#else
  m_fileLogPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation).toStdString();
  m_fileLogPath += "/.local/share/VSCP/btest/logs/btest.log";
#endif
  m_maxFileLogSize  = 5242880;
  m_maxFileLogFiles = 7;

  m_bEnableConsoleLog = true;
  m_consoleLogLevel   = spdlog::level::trace;
  m_consoleLogPattern = "[btest] [%^%l%$] %v";

  pClient = nullptr;

  sem_init(&m_semReceiveQueue, 0, 0);

  if (0 != pthread_mutex_init(&m_mutexReceiveQueue, NULL)) {
    spdlog::error("\n mutex init of input mutex has failed\n");
  }
}

///////////////////////////////////////////////////////////////////////////////
// ~btest
//

btest::~btest()
{
  int rv;

  writeProgramSettings();

  m_bRun = false;
  if (m_threadWork) {
    pthread_join(m_threadWork, NULL);
  }

  // if (VSCP_ERROR_SUCCESS != (rv = vscpboot_release_hardware())) {
  //   spdlog::error("Failed to release hardware rv={}", rv);
  //   return;
  // }

  while (!m_inqueue.isEmpty()) {
    vscpEventEx* pex = m_inqueue.dequeue();
    delete pex;
  }

  spdlog::drop_all();
  spdlog::shutdown();

  sem_destroy(&m_semReceiveQueue);
  pthread_mutex_destroy(&m_mutexReceiveQueue);
}

///////////////////////////////////////////////////////////////////////////////
// startWorkerThread *
//

int
btest::startWorkerThread(void)
{
  // Start the bootloader workerthread
  m_bRun = true; // Workerthread should run, run, run...
  if (pthread_create(&m_threadWork, NULL, workerThread, this)) {
    spdlog::critical("BTEST: Failed to start workerthread");
    return VSCP_ERROR_ERROR;
  }
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// loadProgramSettings
//

void
btest::loadProgramSettings(void)
{
  QString str;

  QSettings settings(m_configFolder, QSettings::NativeFormat);

  spdlog::debug("load config path {}\n", m_configFolder.toStdString());

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
// writeProgramSettings
//

void
btest::writeProgramSettings()
{
  QSettings settings(m_configFolder, QSettings::NativeFormat);

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
// loadFirmwareConfig
//

int
btest::loadFirmwareConfig(QString& path)
{
  int rv = VSCP_ERROR_SUCCESS;
  json j;

  spdlog::debug("Reading json firmware configuration {0}", path.toStdString());

  try {
    std::ifstream ifs(path.toStdString(), std::ifstream::in);
    ifs >> j;
    ifs.close();
  }
  catch (...) {
    spdlog::error("Parse-firmware-config: Failed to parse JSON configuration.");
    return false;
  }

  spdlog::trace("Firmware config: <<{}>>", j.dump());

  if (j.contains("interface") && j["interface"].is_object()) {

    // interface type
    if (j["interface"].contains("type") && (j["interface"]["type"].is_string())) {
      std::string iface = j["interface"]["type"];
      m_interface       = iface.c_str();
      m_interface       = m_interface.trimmed();
      m_interface       = m_interface.toLower();
      spdlog::debug("Parse-JSON: interface: {}", m_interface.toStdString());
    }

    // interface type
    if (j["interface"].contains("config") && (j["interface"]["config"].is_string())) {
      std::string cfg = j["interface"]["config"];
      m_config        = cfg.c_str();
      m_configVector.clear();
      vscp_split(m_configVector, cfg, ";");
      int idx = 0;
      for (const auto& cfgopt : m_configVector) {
        spdlog::debug("Parse-JSON: config string item {0}: {1}", idx, cfgopt);
        idx++;
      }
    }

    // Connect timeout
    if (j["interface"].contains("connect-timout")) {
      if (j["interface"]["connect-timout"].is_string()) {
        m_timeoutConnect = vscp_readStringValue(j["interface"]["connect-timout"]);
      }
      else if (j["interface"]["connect-timout"].is_number_integer()) {
        m_timeoutConnect = j["interface"]["connect-timout"];
      }
    }
  }

  // Bootloader defines
  if (j.contains("bootloader") && j["bootloader"].is_object()) {

    // Block count
    if (j["bootloader"].contains("blocks")) {
      if (j["bootloader"]["blocks"].is_string()) {
        m_bootloader_cfg.blockCount = vscp_readStringValue(j["bootloader"]["blocks"]);
      }
      else if (j["bootloader"]["blocks"].is_number_integer()) {
        m_bootloader_cfg.blockCount = j["bootloader"]["blocks"];
      }
    }

    // Block size
    if (j["bootloader"].contains("blocksize")) {
      if (j["bootloader"]["blocksize"].is_string()) {
        m_bootloader_cfg.blockSize = vscp_readStringValue(j["bootloader"]["blocksize"]);
      }
      else if (j["bootloader"]["blocksize"].is_number_integer()) {
        m_bootloader_cfg.blockSize = j["bootloader"]["blocksize"];
      }
    }
  }

  // Firmware defines
  if (j.contains("device") && j["device"].is_object()) {

    json jj = j["device"];

    if (jj.contains("name") && jj["name"].is_string()) {
      std::string str = jj["name"];
      strncpy((char*)m_firmware_cfg.m_deviceName, str.c_str(), 64);
    }

    if (jj.contains("level") && jj["level"].is_number_integer()) {
        switch ((int)jj["level"]) {

          case 2:
            m_firmware_cfg.m_level = VSCP_LEVEL2;
            break;

          default:
          case 1:
            m_firmware_cfg.m_level = VSCP_LEVEL1;
            break;
        }
    }

    if (jj.contains("guid")) {
      if (jj["guid"].is_string()) {
        std::string str = jj["guid"];
        cguid guid(str);
        memcpy(m_firmware_cfg.m_guid, guid.getGUID(), 16);
      }
      else if (jj["guid"].is_array() && (jj["guid"].size() <= 16)) {
        int idx = 0;
        cguid guid;
        for (auto& item : jj["guid"].items()) {
          guid.setAt(idx, (int)item.value());
          idx++;
        }
      }
    }

    if (jj.contains("mdfurl") && jj["mdfurl"].is_string()) {
      std::string str = jj["mdfurl"];
      strncpy((char*)m_firmware_cfg.m_mdfurl, str.c_str(), 32);
    }

    if (jj.contains("bUse16BitNickname") && jj["bUse16BitNickname"].is_boolean()) {
      m_firmware_cfg.m_bUse16BitNickname = jj["bUse16BitNickname"];
    }

    if (jj.contains("nickname")) {
      if (jj["nickname"].is_number_integer()) {
        m_firmware_cfg.m_nickname = jj["nickname"];
      }
      else if (jj["nickname"].is_string()) {
        m_firmware_cfg.m_nickname = vscp_readStringValue(jj["nickname"]);
      }
    }

    if (jj.contains("bootflag")) {
      if (jj["bootflag"].is_number_integer()) {
        m_bootflag = (int)jj["bootflag"];
      }
      else if (jj["bootflag"].is_string()) {
        m_bootflag = vscp_readStringValue(jj["bootflag"]);
      }
    }

    if (jj.contains("hertbeat-interval")) {
      if (jj["hertbeat-interval"].is_number_integer()) {
        m_firmware_cfg.m_interval_heartbeat = jj["hertbeat-interval"];
      }
      else if (jj["hertbeat-interval"].is_string()) {
        m_firmware_cfg.m_interval_heartbeat = vscp_readStringValue(jj["hertbeat-interval"]);
      }
    }

    if (jj.contains("caps-interval")) {
      if ( jj["caps-interval"].is_number_integer()) {
        m_firmware_cfg.m_interval_caps = jj["caps-interval"];
      }
      else if (jj["caps-interval"].is_string()) {
        m_firmware_cfg.m_interval_caps = vscp_readStringValue(jj["caps-interval"]);
      }
    }

    if (jj.contains("bEnableLogging") && jj["bEnableLogging"].is_boolean()) {
      m_firmware_cfg.m_bEnableLogging = jj["bEnableLogging"];
    }

    if (jj.contains("log-id")) {
      if (jj["log-id"].is_number_integer()) {
        m_firmware_cfg.m_log_id = jj["log-id"];
      }
      else if (jj["log-id"].is_string()) {
        m_firmware_cfg.m_log_id = vscp_readStringValue(jj["log-id"]);
      }
    }

    if (jj.contains("bEnableErrorReporting") && jj["bEnableErrorReporting"].is_boolean()) {
      m_firmware_cfg.m_bEnableErrorReporting = jj["bEnableErrorReporting"];
    }

    if (jj.contains("bEnableErrorReporting") && jj["bEnableErrorReporting"].is_boolean()) {
      m_firmware_cfg.m_bSendHighEndServerProbe = jj["bEnableErrorReporting"];
    }

    if (jj.contains("bHighEndServerResponse") && jj["bHighEndServerResponse"].is_boolean()) {
      m_firmware_cfg.m_bHighEndServerResponse = jj["bHighEndServerResponse"];
    }

    if (jj.contains("bEnableWriteProtectedLocations") && jj["bEnableWriteProtectedLocations"].is_boolean()) {
      m_firmware_cfg.m_bEnableWriteProtectedLocations = jj["bEnableWriteProtectedLocations"];
    }

    if (jj.contains("user-id")) {
      if (jj["user-id"].is_number_integer()) {
        m_firmware_cfg.m_userId = (int)jj["user-id"];
      }
      else if (jj["user-id"].is_string()) {
        m_firmware_cfg.m_userId = vscp_readStringValue(jj["user-id"]);
      }
      else if (jj["user-id"].is_array() && (jj["user-id"].size() <= 4)) {
        int idx = 0;
        uint8_t userid[4];
        for (auto& item : jj["user-id"].items()) {
          userid[idx] = (int)item.value();
          idx++;
        }
        m_firmware_cfg.m_userId = construct_unsigned32(userid[0], userid[1], userid[2], userid[3]);
      }
    }

    if (jj.contains("manufactured-id")) {
      if (jj["usemanufacturedr-id"].is_number_integer()) {
        m_firmware_cfg.m_manufacturerId = (int)jj["manufactured-id"];
      }
      else if (jj["manufactured-id"].is_string()) {
        m_firmware_cfg.m_manufacturerId = vscp_readStringValue(jj["manufactured-id"]);
      }
      else if (jj["manufactured-id"].is_array() && (jj["manufactured-id"].size() <= 4)) {
        int idx = 0;
        uint8_t manufacturerid[4];
        for (auto& item : jj["manufactured-id"].items()) {
          manufacturerid[idx] = (int)item.value();
          idx++;
        }
        m_firmware_cfg.m_manufacturerId = construct_unsigned32(manufacturerid[0], manufacturerid[1], manufacturerid[2], manufacturerid[3]);
      }
    }

    if (jj.contains("manufacturer-sub-id")) {
      if (jj["manufacturer-sub-id"].is_number_integer()) {
        m_firmware_cfg.m_manufacturerSubId = (int)jj["manufacturer-sub-id"];
      }
      else if (jj["manufacturer-sub-id"].is_string()) {
        m_firmware_cfg.m_manufacturerSubId = vscp_readStringValue(jj["manufacturer-sub-id"]);
      }
      else if (jj["manufacturer-sub-id"].is_array() && (jj["manufacturer-sub-id"].size() <= 4)) {
        int idx = 0;
        uint8_t manufacturersubid[4];
        for (auto& item : jj["manufacturer-sub-id"].items()) {
          manufacturersubid[idx] = (int)item.value();
          idx++;
        }
        m_firmware_cfg.m_manufacturerSubId = construct_unsigned32(manufacturersubid[0], manufacturersubid[1], manufacturersubid[2], manufacturersubid[3]);
      }
    }

    if (jj.contains("firmware-version") && jj["firmware-version"].is_array() && (jj["firmware-version"].size() <= 3)) {
      int idx = 0;
      for (auto& item : jj["firmware-version"].items()) {
        switch (idx) {
          case 0:
            m_firmware_cfg.m_firmware_major_version = (int)item.value();
            break;

          case 1:
            m_firmware_cfg.m_firmware_minor_version = (int)item.value();
            break;

          case 2:
            m_firmware_cfg.m_firmware_sub_minor_version = (int)item.value();
            break;
        }
        idx++;
      }
    }

    if (jj.contains("bootloader-algorithm")) {
      if ( jj["bootloader-algorithm"].is_number_integer()) {
        m_firmware_cfg.m_bootloader_algorithm = jj["bootloader-algorithm"];
      }
      else if (jj["bootloader-algorithm"].is_string()) {
        m_firmware_cfg.m_bootloader_algorithm = vscp_readStringValue(jj["bootloader-algorithm"]);
      }
    }

    if (jj.contains("standard-device-family-code")) {
      if (jj["standard-device-family-code"].is_number_integer()) {
        m_firmware_cfg.m_standard_device_family_code = (int)jj["standard-device-family-code"];
      }
      else if (jj["standard-device-family-code"].is_string()) {
        m_firmware_cfg.m_standard_device_family_code = vscp_readStringValue(jj["standard-device-family-code"]);
      }
      else if (jj["standard-device-family-code"].is_array() && (jj["standard-device-family-code"].size() <= 4)) {
        int idx = 0;
        uint8_t val[4];
        for (auto& item : jj["standard-device-family-code"].items()) {
          val[idx] = (int)item.value();
          idx++;
        }
        m_firmware_cfg.m_standard_device_family_code = construct_unsigned32(val[0], val[1], val[2], val[3]);
      }
    }

    if (jj.contains("standard-device-type-code")) {
      if (jj["standard-device-type-code"].is_number_integer()) {
        m_firmware_cfg.m_standard_device_family_code = (int)jj["standard-device-type-code"];
      }
      else if (jj["standard-device-type-code"].is_string()) {
        m_firmware_cfg.m_standard_device_family_code = vscp_readStringValue(jj["standard-device-type-code"]);
      }
      else if (jj["standard-device-type-code"].is_array() && (jj["standard-device-type-code"].size() <= 4)) {
        int idx = 0;
        uint8_t val[4];
        for (auto& item : jj["standard-device-type-code"].items()) {
          val[idx] = (int)item.value();
          idx++;
        }
        m_firmware_cfg.m_standard_device_type_code = construct_unsigned32(val[0], val[1], val[2], val[3]);
      }
    }

    if (jj.contains("firmware-device-code")) {
      if (jj["standard-device-type-code"].is_number_integer()) {
        m_firmware_cfg.m_firmware_device_code = (int)jj["firmware-device-code"];
      }
      else if (jj["firmware-device-code"].is_string()) {
        m_firmware_cfg.m_firmware_device_code = vscp_readStringValue(jj["firmware-device-code"]);
      }
      else if (jj["firmware-device-code"].is_array() && (jj["firmware-device-code"].size() <= 2)) {
        int idx = 0;
        uint8_t val[2];
        for (auto& item : jj["firmware-device-code"].items()) {
          val[idx] = (int)item.value();
          idx++;
        }
        m_firmware_cfg.m_firmware_device_code = construct_unsigned16(val[0], val[1]);
      }
    }

    if (jj.contains("ip-addr")) {
      if (jj["ip-addr"].is_number_integer()) {
        // Numerical ipv4 address
        uint32_t ipaddr            = (int)jj["ip-addr"];
        m_firmware_cfg.m_ipaddr[0] = *((uint8_t*)&ipaddr);
        m_firmware_cfg.m_ipaddr[1] = *((uint8_t*)&ipaddr + 1);
        m_firmware_cfg.m_ipaddr[2] = *((uint8_t*)&ipaddr + 2);
        m_firmware_cfg.m_ipaddr[3] = *((uint8_t*)&ipaddr + 3);
      }
      else if (jj["ip-addr"].is_string()) {
        std::string str = jj["ip-addr"];
        std::deque<std::string> parts;
        // ipv4 address
        vscp_split(parts, str, ".");
        if (parts.size()) {
          int idx = 0;
          for (const auto& x : parts) {
            m_firmware_cfg.m_ipaddr[idx] = atoi(x.c_str());
            idx++;
          }
        }
        else {
          // ipv6 address
          vscp_split(parts, str, ":");
          if (parts.size()) {
            int idx = 0;
            for (const auto& x : parts) {
              std::string str                  = "0x" + x;
              uint16_t val                     = vscp_readStringValue(str);
              m_firmware_cfg.m_ipaddr[idx]     = (val >> 8) & 0xff;
              m_firmware_cfg.m_ipaddr[idx + 1] = val & 0xff;
              idx += 2;
            }
          }
        }
      }
      else if (jj["ip-addr"].is_array() && (jj["ip-addr"].size() <= 16)) {
        int idx = 0;
        for (auto& item : jj["ip-addr"].items()) {
          m_firmware_cfg.m_ipaddr[idx] = (int)item.value();
          idx++;
        }
      }
    }

  } // device

  return rv;
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
// receiveCallback
//

void
btest::receiveCallback(vscpEventEx& ex, void* pobj)
{
  vscpEventEx* pexnew = new vscpEventEx;
  if (nullptr != pexnew) {
    pexnew->sizeData = 0;
    vscp_copyEventEx(pexnew, &ex);
    pthread_mutex_lock(&m_mutexReceiveQueue);
    m_inqueue.append(pexnew);
    sem_post(&m_semReceiveQueue);
    pthread_mutex_unlock(&m_mutexReceiveQueue);
  }

  spdlog::trace("Data received: {0:X}:{1:X} size={2}\n", ex.vscp_class, ex.vscp_type, ex.sizeData);
  // emit dataReceived(&ex);

  // Alternative method for reference
  // CFrmSession* pSession = (CFrmSession*)pobj;
  // pSession->threadReceive(pevnew);
}

///////////////////////////////////////////////////////////////////////////////
//                  Handlers for VSCP Firmware callbacks
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// readRegister
//

int
btest::readRegister(uint16_t page, uint32_t reg, uint8_t* pval)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// writeRegister
//

int
btest::writeRegister(uint16_t page, uint32_t reg, uint8_t val)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receivedSegCtrlHeartBeat
//

int
btest::receivedSegCtrlHeartBeat(uint16_t segcrc, uint32_t tm)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// newNodeOnline
//

int
btest::newNodeOnline(uint16_t nickname)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// newNodeOnline
//

int
btest::newNodeOnline(cguid& guid)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// standardRegHasChanged
//

int
btest::standardRegHasChanged(uint32_t stdreg)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// reportDM
//
int
btest::reportDM(void)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// reportEventsOfInterest
//
int
btest::reportEventsOfInterest(void)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// sendEmbeddedMDF
//
// Send embedded MDF if one is available
//

int
btest::sendEmbeddedMDF(void)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// getMdfUrl
//

int
btest::getMdfUrl(uint8_t* const purl)
{
  uint8_t buf[10];
  memcpy(purl, buf, 10);
  return VSCP_ERROR_SUCCESS;
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

    if (0 == m_configVector.size()) {
      spdlog::error("vscpboot_init_hardware: no socketcan interface given.");
      return VSCP_ERROR_INIT_MISSING;
    }

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

    using namespace std::placeholders;
    auto cb = std::bind(&btest::receiveCallback, this, _1, _2);
    // lambda version for reference
    // auto cb = [this](auto a, auto b) { this->receiveCallback(a, b); };
    pClient->setCallbackEx(cb, this);

    if (VSCP_ERROR_SUCCESS != (rv = pClient->connect())) {
      delete (vscpClientSocketCan*)pClient;
      pClient = nullptr;
      spdlog::error("Unable to initialize socketcan client {0} rv={1}", m_interface.toStdString(), rv);
      return VSCP_ERROR_HARDWARE;
    }

    uint32_t timeout = vscp_getMsTimeStamp();
    while (!pClient->isConnected()) {
      spdlog::trace("Waiting for connect...");
      sleep(1);
      if ((vscp_getMsTimeStamp() - timeout) > m_timeoutConnect) {
        pClient->disconnect();
        return VSCP_ERROR_TIMEOUT;
      }
    };
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
  return m_bootflag;
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
  m_bootflag = bootflag;
  return VSCP_ERROR_SUCCESS;
}

/*!
  Reboot the board. This method should never return.
*/
void
btest::vscpboot_reboot(void)
{
  // We restart the workerthread
}

/*!
  Get configuration
*/
vscpboot_config_t*
btest::vscpboot_getConfig(void)
{
  return &m_bootloader_cfg;
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
btest::vscpboot_sendEventEx(vscpEventEx* pex)
{
  return pClient->send(*pex);
}

/*!
  Get VSCP event (Block until event is received)
  -----------------------------------------------------------
  IMPORTANT!
  This routine should translate all VSCP_CLASS2_LEVEL1_PROTOCOL
  events to VSCP_CLASS1_PROTOCOL events.
  -----------------------------------------------------------
  @param Pointer to VSCP event structure.
   @return VSCP_ERROR_SUCCESS on success
*/
int
btest::vscpboot_getEventEx(vscpEventEx* pex)
{
  int rv;

RCVLOOP:
  if ((-1 == (rv = vscp_sem_wait(&m_semReceiveQueue, 10))) &&
      errno == ETIMEDOUT) {
    // If app. should run continue blocking operation
    if (m_bRun) {
      goto RCVLOOP;
    }
  }

  // Return if error
  if (rv) {
    return VSCP_ERROR_ERROR;
  }

  // If this is a proxy event then translate to standard event
  if ((pex->vscp_class >= 512) && ((pex->vscp_class < 1024))) {
    pex->vscp_class -= 512;                                // Standard level I class
    memcpy(pex->data, pex->data + 16, pex->sizeData - 16); // Remove proxy interface
  }

  pthread_mutex_lock(&m_mutexReceiveQueue);
  if (m_inqueue.size()) {
    pex = m_inqueue.dequeue();
    pthread_mutex_unlock(&m_mutexReceiveQueue);
    spdlog::trace("Event {0:x}:{1:x}", pex->vscp_class, pex->vscp_type);
  }
  else {
    pthread_mutex_unlock(&m_mutexReceiveQueue);
    spdlog::trace("No events to fetch");
    return VSCP_ERROR_FIFO_EMPTY;
  }
  return VSCP_ERROR_SUCCESS;
}

//////////////////////////////////////////////////////////////////////
//                     Workerthread -
//////////////////////////////////////////////////////////////////////

void*
workerThread(void* pData)
{
  int rv;
  fd_set rdfs;
  struct timeval tv;

  btest* pbtest = (btest*)pData;
  if (NULL == pbtest) {
    spdlog::error("btest: No object data object supplied for worker thread");
    return NULL;
  }

  /*!
    Run bootloader if bootflag is non-zero
    otherwise run the simulated firmware
  */
  if (pbtest->vscpboot_getBootFlag()) {
    spdlog::info("Starting simulation software in bootloader mode");
    vscpboot_loader();
  }
  else {

    // * * * Simulated firmware * * *
    spdlog::info("Starting simulation software in firmware mode");

    // Initialize the firmware
    if (VSCP_ERROR_SUCCESS != (rv = vscp_frmw2_init(&pbtest->m_firmware_cfg))) {
      spdlog::error("workerthread: Failed to initialize firmware.");
      return NULL;
    }
  }

  return NULL;
}
