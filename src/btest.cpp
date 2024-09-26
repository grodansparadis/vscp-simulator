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

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifdef WIN32
#include <pch.h>
#endif

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
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

#include "mainwindow.h"

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

#include "spdlog/fmt/bin_to_hex.h"
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

  // Set to known state
  memset(&m_firmware_cfg, 0, sizeof(vscp_frmw2_firmware_config_t));

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
  std::cout << "Reading configuration file from " << m_configFolder.toStdString() << std::endl;

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

  m_pClient = nullptr;

  sem_init(&m_semReceiveQueue, 0, 0);

  if (0 != pthread_mutex_init(&m_mutexReceiveQueue, NULL)) {
    spdlog::error("\n mutex init of input mutex has failed\n");
  }

  m_nSimulation = 0; // Code for simulation (zero is unknow/uninitialized)
  m_pSim        = nullptr;
}

///////////////////////////////////////////////////////////////////////////////
// ~btest
//

btest::~btest()
{
  int rv;

  writeProgramSettings();

  if (VSCP_ERROR_SUCCESS != (rv = stopWorkerThread())) {
    spdlog::error("Failed to stop workerthread rv=%d", rv);
  }

  // if (VSCP_ERROR_SUCCESS != (rv = vscpboot_release_hardware())) {
  //   spdlog::error("Failed to release hardware rv={}", rv);
  //   return;
  // }

  while (!m_inqueue.isEmpty()) {
    vscpEventEx* pex = m_inqueue.dequeue();
    delete pex;
  }

  // If event array defined - delete it
  if (m_firmware_cfg.m_pEventsOfInterest) {
    delete[] m_firmware_cfg.m_pEventsOfInterest;
  }

  // Clear up simulation data
  if (nullptr != m_pSim) {
    switch (m_nSimulation) {
      case 1:
      default: {
        delete (simulation1_t*)m_pSim;
      } break;
    }
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
// stopWorkerThread
//

int
btest::stopWorkerThread(void)
{
  // Start the bootloader workerthread
  m_bRun = false; // Workerthread should end it's life
  if (m_threadWork) {
    pthread_join(m_threadWork, NULL);
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
  std::cout << "Reading json firmware configuration " << path.toStdString() << std::endl;

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
      if (jj["caps-interval"].is_number_integer()) {
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

    // if (jj.contains("bEnableErrorReporting") && jj["bEnableErrorReporting"].is_boolean()) {
    //   m_firmware_cfg.m_bSendHighEndServerProbe = jj["bEnableErrorReporting"];
    // }

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
      if (jj["bootloader-algorithm"].is_number_integer()) {
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

    /*
    "dm-rows":10,
    "dm-row-size":8,
    "dm-offset":0,
    "dm-page-start":1,
    "dm-data": [1,2,3,4,5,6,7,8,9],
    */

    if (jj.contains("dm-rows")) {
      if (jj["dm-rows"].is_number_integer()) {
        m_firmware_cfg.m_nDmRows = jj["dm-rows"];
      }
      else if (jj["dm-rows"].is_string()) {
        m_firmware_cfg.m_nDmRows = vscp_readStringValue(jj["dm-rows"]);
      }
    }

    if (VSCP_LEVEL1 == m_firmware_cfg.m_level) {
      m_firmware_cfg.m_sizeDmRow = 8;
    }
    else {
      if (jj.contains("dm-size")) {
        if (jj["dm-size"].is_number_integer()) {
          m_firmware_cfg.m_sizeDmRow = jj["dm-size"];
        }
        else if (jj["dm-size"].is_string()) {
          m_firmware_cfg.m_sizeDmRow = vscp_readStringValue(jj["dm-size"]);
        }
      }
    }

    if (jj.contains("dm-offset")) {
      if (jj["dm-offset"].is_number_integer()) {
        m_firmware_cfg.m_regOffsetDm = jj["dm-offset"];
      }
      else if (jj["dm-offset"].is_string()) {
        m_firmware_cfg.m_regOffsetDm = vscp_readStringValue(jj["dm-offset"]);
      }
    }

    if (jj.contains("dm-page-start")) {
      if (jj["dm-page-start"].is_number_integer()) {
        m_firmware_cfg.m_pageDm = jj["dm-page-start"];
      }
      else if (jj["dm-page-start"].is_string()) {
        m_firmware_cfg.m_pageDm = vscp_readStringValue(jj["dm-page-start"]);
      }
    }

    if (jj.contains("simulation") && jj["simulation"].is_number_integer()) {
      m_nSimulation = jj["simulation"];
    }

  } // device

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// getMainWindow
//

MainWindow*
btest::getMainWindow()
{
  foreach (QWidget* w, qApp->topLevelWidgets()) {
    if (QMainWindow* mainWin = qobject_cast<QMainWindow*>(w)) {
      return (MainWindow*)mainWin;
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

  spdlog::trace("[receiveCallback] Data received: {0:X}:{1:X} size={2}\n", ex.vscp_class, ex.vscp_type, ex.sizeData);
  // emit dataReceived(&ex);

  // Alternative method for reference
  // CFrmSession* pSession = (CFrmSession*)pobj;
  // pSession->threadReceive(pevnew);
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
btest::getEventEx(vscpEventEx** pex)
{
  int rv;

  if (-1 == (rv = vscp_sem_wait(&m_semReceiveQueue, 100))) {
    if (errno == ETIMEDOUT) {
      return VSCP_ERROR_TIMEOUT;
    }
    else {
      return VSCP_ERROR_ERROR;
    }
  }

  pthread_mutex_lock(&m_mutexReceiveQueue);
  if (m_inqueue.size()) {
    *pex = m_inqueue.dequeue();
    pthread_mutex_unlock(&m_mutexReceiveQueue);
    spdlog::trace("[btest::getEventEx] {0:X}:{1:X}", (*pex)->vscp_class, (*pex)->vscp_type);

    // If this is a proxy event then translate to standard event
    if (((*pex)->vscp_class >= 512) && (((*pex)->vscp_class < 1024))) {
      (*pex)->vscp_class -= 512;                                      // Standard level I class
      memcpy((*pex)->data, (*pex)->data + 16, (*pex)->sizeData - 16); // Remove proxy interface
    }
  }
  else {
    pthread_mutex_unlock(&m_mutexReceiveQueue);
    // spdlog::trace("[btest::getEventEx] No events to fetch");
    return VSCP_ERROR_FIFO_EMPTY;
  }
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//                           SIMULATION 1
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// initSimulationData
//

bool
btest::initSimulationData(void)
{
  switch (m_nSimulation) {

    default:
      // Simulation is not defined we use sim1
      spdlog::error("Invalid simulation set. Will use sumulation 1");

    case 1: {

      m_pSim = new simulation1_t;
      memset(m_pSim, 0, sizeof(simulation1_t));
      m_firmware_cfg.m_pEventsOfInterest              = nullptr;
      ((simulation1_t*)m_pSim)->m_background_color[0] = 0xff;
      ((simulation1_t*)m_pSim)->m_background_color[1] = 0xff;
      ((simulation1_t*)m_pSim)->m_background_color[2] = 0xff;

      // Page 0
      for (int i = 0; i <= 86; i++) {
        m_regmap[i] = nullptr; // No table item yet (inserted by mainwindow)
      }

      // Standard registers
      for (int i = 128; i <= 255; i++) {
        m_regmap[i] = nullptr; // No table item yet (inserted by mainwindow)
      }

      // page 1
      for (int i = 0; i <= 3; i++) {
        m_regmap[(1 << 16) + i] = nullptr; // No table item yet (inserted by mainwindow)
      }

      // page 2
      for (int i = 0; i <= 3; i++) {
        m_regmap[(2 << 16) + i] = nullptr; // No table item yet (inserted by mainwindow)
      }

      // emit updateRegister(82, 0, 0xff);
      // emit updateRegister(83, 0, 0xff);
      // emit updateRegister(84, 0, 0xff);

    } break;
  }

  return true;
}

///////////////////////////////////////////////////////////////////////////////
// readRegister_sim1
//

int
btest::readRegister_sim1(uint16_t page, uint32_t reg, uint8_t* pval)
{
  int rv = VSCP_ERROR_SUCCESS;

  // Pointer to value must be valid
  if (nullptr == pval) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  // Pointer to simulation storage must be valid
  if (nullptr == m_pSim) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  simulation1_t* psim1 = (simulation1_t*)m_pSim;

  switch (page) {

    case 0: {
      switch (reg) {

        case 0: // zone
          *pval = psim1->m_reg_zone;
          break;

        case 1:  // S0 subzone
        case 2:  // S1 subzone
        case 3:  // S2 subzone
        case 4:  // S3 subzone
        case 5:  // S4 subzone
        case 6:  // S5 subzone
        case 7:  // S6 subzone
        case 8:  // S7 subzone
        case 9:  // S8 subzone
        case 10: // S9 subzone
          *pval = psim1->m_reg_subzone_S[reg - 1];
          break;

        case 11: // C0 subzone
        case 12: // C1 subzone
        case 13: // C2 subzone
        case 14: // C3 subzone
        case 15: // C4 subzone
        case 16: // C5 subzone
        case 17: // C6 subzone
        case 18: // C7 subzone
        case 19: // C8 subzone
        case 20: // C9 subzone
          *pval = psim1->m_reg_subzone_C[reg - 11];
          break;

        case 21: // R0 subzone
        case 22: // R1 subzone
        case 23: // R2 subzone
        case 24: // R3 subzone
        case 25: // R4 subzone
        case 26: // R5 subzone
        case 27: // R6 subzone
        case 28: // R7 subzone
        case 29: // R8 subzone
        case 30: // R9 subzone
          *pval = psim1->m_reg_subzone_R[reg - 21];
          break;

        case 31: // Slider 0 subzone
        case 32: // Slider 1 subzone
        case 33: // Slider 2 subzone
        case 34: // Slider 3 subzone
        case 35: // Slider 4 subzone
        case 36: // Slider 5 subzone
        case 37: // Slider 6 subzone
        case 38: // Slider 7 subzone
        case 39: // Slider 8 subzone
        case 40: // Slider 9 subzone
          *pval = psim1->m_reg_value_slider[reg - 31];
          break;

        case 41: // S0 value
        case 42: // S1 value
        case 43: // S2 value
        case 44: // S3 value
        case 45: // S4 value
        case 46: // S5 value
        case 47: // S6 value
        case 48: // S7 value
        case 49: // S8 value
        case 50: // S9 value
          *pval = psim1->m_reg_value_S[reg - 41];
          break;

        case 51: // C0 value
        case 52: // C1 value
        case 53: // C2 value
        case 54: // C3 value
        case 55: // C4 value
        case 56: // C5 value
        case 57: // C6 value
        case 58: // C7 value
        case 59: // C8 value
        case 60: // C9 value
          *pval = psim1->m_reg_value_C[reg - 51];
          break;

        case 61: // R0 value
        case 62: // R1 value
        case 63: // R2 value
        case 64: // R3 value
        case 65: // R4 value
        case 66: // R5 value
        case 67: // R6 value
        case 68: // R7 value
        case 69: // R8 value
        case 70: // R9 value
          *pval = psim1->m_reg_value_R[reg - 61];
          break;

        case 71: // Slider 0 value
        case 72: // Slider 1 value
        case 73: // Slider 2 value
        case 74: // Slider 3 value
        case 75: // Slider 4 value
        case 76: // Slider 5 value
        case 77: // Slider 6 value
        case 78: // Slider 7 value
        case 79: // Slider 8 value
        case 80: // Slider 9 value
          *pval = psim1->m_reg_value_slider[reg - 71];
          break;

        case 81: // Place holder for unsigned int forming RGB r-var
          // Allways read as zero
          *pval = 0;
          break;

        case 82: // background color R
        case 83: // background color G
        case 84: // background color B
          *pval = psim1->m_background_color[reg - 82];
          break;

        case 85: // period for periodic measurement event
          *pval = psim1->m_period_measurement_event;
          break;

        case 86: // coding for periodic measurement event
          *pval = psim1->m_coding_measurement_event;
          break;

        default:
          if ((reg >= 0x1000) && (reg < 0x1080)) {
            *pval = psim1->m_dm[reg - 0x1000];
          }
          else {
            spdlog::warn("Register index out of bounds reg={}", reg);
            rv = VSCP_ERROR_INDEX_OOB;
          }
          break;
      }
    } break;

    // Only for level 1 - DM
    case 1: {
      if (reg < 80) {
        *pval = psim1->m_dm[reg];
      }
    } break;

    default:
      spdlog::warn("Register index out of bounds reg={}", reg);
      rv = VSCP_ERROR_INDEX_OOB;
      break;
  }

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// writeRegister_sim1
//

int
btest::writeRegister_sim1(uint16_t page, uint32_t reg, uint8_t val)
{
  int rv = VSCP_ERROR_SUCCESS;

  // Pointer to simulation storage must be valid
  if (nullptr == m_pSim) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  simulation1_t* psim1 = (simulation1_t*)m_pSim;

  switch (page) {

    case 0: {
      switch (reg) {

        case 0: // zone
          psim1->m_reg_zone = val;
          break;

        case 1:  // S0 subzone
        case 2:  // S1 subzone
        case 3:  // S2 subzone
        case 4:  // S3 subzone
        case 5:  // S4 subzone
        case 6:  // S5 subzone
        case 7:  // S6 subzone
        case 8:  // S7 subzone
        case 9:  // S8 subzone
        case 10: // S9 subzone
          psim1->m_reg_subzone_S[reg - 1] = val;
          break;

        case 11: // C0 subzone
        case 12: // C1 subzone
        case 13: // C2 subzone
        case 14: // C3 subzone
        case 15: // C4 subzone
        case 16: // C5 subzone
        case 17: // C6 subzone
        case 18: // C7 subzone
        case 19: // C8 subzone
        case 20: // C9 subzone
          psim1->m_reg_subzone_C[reg - 11] = val;
          checkValueChanged(reg - 11, val);
          break;

        case 21: // R0 subzone
        case 22: // R1 subzone
        case 23: // R2 subzone
        case 24: // R3 subzone
        case 25: // R4 subzone
        case 26: // R5 subzone
        case 27: // R6 subzone
        case 28: // R7 subzone
        case 29: // R8 subzone
        case 30: // R9 subzone
          psim1->m_reg_subzone_R[reg - 21] = val;
          radioValueChanged(reg - 21, val);
          break;

        case 31:   // Slider 0 subzone
        case 32:   // Slider 1 subzone
        case 33:   // Slider 2 subzone
        case 34:   // Slider 3 subzone
        case 35:   // Slider 4 subzone
        case 36:   // Slider 5 subzone
        case 37:   // Slider 6 subzone
        case 38:   // Slider 7 subzone
        case 39:   // Slider 8 subzone
        case 40: { // Slider 9 subzone
          psim1->m_reg_value_slider[reg - 31] = val;
          emit sliderValueChanged(reg - 31, val);
        } break;

        case 41: // S0 value
        case 42: // S1 value
        case 43: // S2 value
        case 44: // S3 value
        case 45: // S4 value
        case 46: // S5 value
        case 47: // S6 value
        case 48: // S7 value
        case 49: // S8 value
        case 50: // S9 value
          psim1->m_reg_value_S[reg - 41] = val;
          break;

        case 51: // C0 value
        case 52: // C1 value
        case 53: // C2 value
        case 54: // C3 value
        case 55: // C4 value
        case 56: // C5 value
        case 57: // C6 value
        case 58: // C7 value
        case 59: // C8 value
        case 60: // C9 value
          psim1->m_reg_value_C[reg - 51] = val;
          checkValueChanged(reg - 51, val);
          break;

        case 61: // R0 value
        case 62: // R1 value
        case 63: // R2 value
        case 64: // R3 value
        case 65: // R4 value
        case 66: // R5 value
        case 67: // R6 value
        case 68: // R7 value
        case 69: // R8 value
        case 70: // R9 value
          psim1->m_reg_value_R[reg - 61] = val;
          radioValueChanged(reg - 61, val);
          break;

        case 71: // Slider 0 value
        case 72: // Slider 1 value
        case 73: // Slider 2 value
        case 74: // Slider 3 value
        case 75: // Slider 4 value
        case 76: // Slider 5 value
        case 77: // Slider 6 value
        case 78: // Slider 7 value
        case 79: // Slider 8 value
        case 80: // Slider 9 value
          psim1->m_reg_value_slider[reg - 71] = val;
          sliderValueChanged(reg - 71, val);
          break;

        case 81: // Placeholder for 32-bit int RGB r-var
          break;

        case 82:   // background color R
        case 83:   // background color G
        case 84: { // background color B
          psim1->m_background_color[reg - 82] = val;
          spdlog::info("Set background color {0:02X}{1:02X}{2:02X}",
                       psim1->m_background_color[0],
                       psim1->m_background_color[1],
                       psim1->m_background_color[2]);
          uint32_t color = construct_unsigned32(0, psim1->m_background_color[0], psim1->m_background_color[1], psim1->m_background_color[2]);
          emit backgroundColorChanged(color);
        } break;

        case 85: // period for status event
          psim1->m_period_measurement_event = val;
          break;

        case 86: // coding for status event
          psim1->m_coding_measurement_event = val;
          break;

        default:
          if ((reg >= 0x1000) && (reg < 0x1080)) {
            psim1->m_dm[reg - 0x1000] = val;
          }
          else {
            spdlog::warn("Register index out of bounds reg={}", reg);
            rv = VSCP_ERROR_INDEX_OOB;
          }
          break;
      }
    } break;

    // Only for level 1 - DM
    case 1: {
      if (reg < 80) {
        psim1->m_dm[reg] = val;
      }
    } break;

    default:
      spdlog::warn("Register index out of bounds reg={}", reg);
      rv = VSCP_ERROR_INDEX_OOB;
      break;
  }

  emit updateRegister(reg, page, val);

  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// reportDM_sim1
//

int
btest::reportDM_sim1(void)
{
  return VSCP_ERROR_SUCCESS;
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
  switch (m_nSimulation) {
    case 0:
    default:
      return readRegister_sim1(page, reg, pval);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// writeRegister
//

int
btest::writeRegister(uint16_t page, uint32_t reg, uint8_t val)
{
  switch (m_nSimulation) {
    case 0:
    default:
      return writeRegister_sim1(page, reg, val);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// writeSliderValue
//

int
btest::writeSliderValue(uint8_t idx, int value)
{
  switch (m_nSimulation) {
    case 0:
    default:
      return writeSliderValue_sim1(idx, value);
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// writeSliderValue_sim1
//

int
btest::writeSliderValue_sim1(uint8_t idx, int value)
{
  // Pointer to simulation storage must be valid
  if (nullptr == m_pSim) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if ((idx > 9) || (idx < 0)) {
    return VSCP_ERROR_PARAMETER;
  }

  simulation1_t* psim1 = (simulation1_t*)m_pSim;

  // Save value
  psim1->m_reg_value_slider[idx] = value;

  /*
    If nickname is not set we don't allow any active
    functionality until it is
  */
  if (VSCP_ADDRESS_NEW_NODE == m_firmware_cfg.m_nickname) {
    return VSCP_ERROR_INIT_MISSING;
  }

  // Send event
  vscpEventEx ex;

  memset(ex.data, 0, 512);
  ex.head = VSCP_PRIORITY_NORMAL;
  memcpy(ex.GUID, m_firmware_cfg.m_guid, 16);
  /*!
    If level I we use the GUID as a space for the nickname
  */
  if (VSCP_LEVEL1 == m_firmware_cfg.m_level) {
    ex.GUID[14]   = (m_firmware_cfg.m_nickname >> 8) & 0xff;
    ex.GUID[15]   = m_firmware_cfg.m_nickname & 0xff;
    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    ex.vscp_type  = VSCP_TYPE_MEASUREMENT_RELATIVE_LEVEL;
    ex.sizeData   = 2;
    ex.data[0]    = 0x60 + idx;
    ex.data[1]    = value;
  }
  else {
    ex.vscp_class = VSCP_CLASS1_MEASUREMENT;
    ex.vscp_type  = VSCP_TYPE_MEASUREMENT_RELATIVE_LEVEL;
    ex.sizeData   = 2;
    ex.data[0]    = 0x60 + idx;
    ex.data[1]    = value;
  }

  return m_pClient->send(ex);
}

///////////////////////////////////////////////////////////////////////////////
// reportDM
//
int
btest::reportDM(void)
{
  switch (m_nSimulation) {
    case 0:
    default:
      return reportDM_sim1();
  }

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
  switch (m_nSimulation) {
    case 0:
    default:
      break;
  }

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
// receivedSegCtrlHeartBeat
//

int
btest::receivedSegCtrlHeartBeat(uint16_t segcrc, uint32_t tm)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// standardRegHasChanged
//

int
btest::standardRegHasChanged(uint32_t stdreg)
{
  uint8_t value = 0;
  uint8_t reg   = stdreg & 0xff;

  // Fet standard register value
  switch (reg) {

    case VSCP_STD_REGISTER_ALARM_STATUS:
      value = m_firmware_cfg.m_alarm_status;
      break;

    case VSCP_STD_REGISTER_MAJOR_VERSION:
      value = m_firmware_cfg.m_vscp_major_version;
      break;

    case VSCP_STD_REGISTER_MINOR_VERSION:
      value = m_firmware_cfg.m_vscp_minor_version;
      break;

    case VSCP_STD_REGISTER_ERROR_COUNTER:
      value = m_firmware_cfg.m_errorCounter;
      break;

    case VSCP_STD_REGISTER_USER_ID:
      value = (m_firmware_cfg.m_userId >> 24) & 0xff;
      break;

    case VSCP_STD_REGISTER_USER_ID + 1:
      value = (m_firmware_cfg.m_userId >> 16) & 0xff;
      break;

    case VSCP_STD_REGISTER_USER_ID + 2:
      value = (m_firmware_cfg.m_userId >> 8) & 0xff;
      break;

    case VSCP_STD_REGISTER_USER_ID + 3:
      value = m_firmware_cfg.m_userId & 0xff;
      break;

    case VSCP_STD_REGISTER_USER_MANDEV_ID:
      value = (m_firmware_cfg.m_manufacturerId >> 24) & 0xff;
      break;
    case VSCP_STD_REGISTER_USER_MANDEV_ID + 1:
      value = (m_firmware_cfg.m_manufacturerId >> 16) & 0xff;
      break;
    case VSCP_STD_REGISTER_USER_MANDEV_ID + 2:
      value = (m_firmware_cfg.m_manufacturerId >> 8) & 0xff;
      break;
    case VSCP_STD_REGISTER_USER_MANDEV_ID + 3:
      value = m_firmware_cfg.m_manufacturerId & 0xff;
      break;

    case VSCP_STD_REGISTER_USER_MANSUBDEV_ID:
      value = (m_firmware_cfg.m_manufacturerSubId >> 24) & 0xff;
      break;
    case VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 1:
      value = (m_firmware_cfg.m_manufacturerSubId >> 16) & 0xff;
      break;
    case VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 2:
      value = (m_firmware_cfg.m_manufacturerSubId >> 8) & 0xff;
      break;
    case VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 3:
      value = m_firmware_cfg.m_manufacturerSubId & 0xff;
      break;

      // See below
      // case VSCP_STD_REGISTER_NICKNAME_ID_LSB:
      //  break;

    case VSCP_STD_REGISTER_PAGE_SELECT_MSB:
      value = (m_firmware_cfg.m_page_select >> 16) & 0xff;
      break;

    case VSCP_STD_REGISTER_PAGE_SELECT_LSB:
      value = m_firmware_cfg.m_page_select & 0xff;
      break;

    case VSCP_STD_REGISTER_FIRMWARE_MAJOR:
      value = m_firmware_cfg.m_firmware_major_version;
      break;

    case VSCP_STD_REGISTER_FIRMWARE_MINOR:
      value = m_firmware_cfg.m_firmware_minor_version;
      break;

    case VSCP_STD_REGISTER_FIRMWARE_SUBMINOR:
      value = m_firmware_cfg.m_firmware_sub_minor_version;
      break;

    case VSCP_STD_REGISTER_BOOT_LOADER:
      value = m_firmware_cfg.m_bootloader_algorithm;
      break;

    case VSCP_STD_REGISTER_BUFFER_SIZE:
      value = 0;
      break;

    case VSCP_STD_REGISTER_PAGES_COUNT:
      value = 0;
      break;

    case VSCP_STD_REGISTER_FAMILY_CODE:
      value = m_firmware_cfg.m_standard_device_family_code;
      break;

    case VSCP_STD_REGISTER_DEVICE_TYPE:
      value = m_firmware_cfg.m_standard_device_type_code;
      break;

    case VSCP_STD_REGISTER_NODE_RESET:
      value = 0;
      break;

    case VSCP_STD_REGISTER_FIRMWARE_CODE_MSB:
      value = (m_firmware_cfg.m_firmware_device_code >> 16) & 0xff;
      break;

    case VSCP_STD_REGISTER_FIRMWARE_CODE_LSB:
      value = m_firmware_cfg.m_firmware_device_code & 0xff;
      break;

    case VSCP_STD_REGISTER_NICKNAME_ID_LSB:
      value = (m_firmware_cfg.m_nickname >> 16) & 0xff;
      break;

    case VSCP_STD_REGISTER_NICKNAME_ID_MSB:
      value = (m_firmware_cfg.m_nickname >> 16) & 0xff;
      break;

    default:
      if ((reg >= VSCP_STD_REGISTER_GUID) && (reg < VSCP_STD_REGISTER_GUID + 16)) {
        value = m_firmware_cfg.m_guid[reg - VSCP_STD_REGISTER_GUID];
      }
      else if ((reg >= VSCP_STD_REGISTER_DEVICE_URL) && (reg < VSCP_STD_REGISTER_DEVICE_URL + 32)) {
        value = m_firmware_cfg.m_mdfurl[reg - VSCP_STD_REGISTER_DEVICE_URL];
      }
      break;
  }

  emit updateRegister(reg, 0, value);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// buttonPress
//

int
btest::buttonPress(int idx)
{
  switch (m_nSimulation) {
    case 0:
    default:
      return buttonPress_sim1(idx);
      break;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// buttonPress_sim1
//

int
btest::buttonPress_sim1(int idx)
{
  // Pointer to simulation storage must be valid
  if (nullptr == m_pSim) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if ((idx > 9) || (idx < 0)) {
    return VSCP_ERROR_PARAMETER;
  }

  simulation1_t* psim1 = (simulation1_t*)m_pSim;

  // Send event
  vscpEventEx ex;

  memset(ex.data, 0, 512);
  ex.head = VSCP_PRIORITY_NORMAL;
  memcpy(ex.GUID, m_firmware_cfg.m_guid, 16);
  /*!
    If level I we use the GUID as a space for the nickname
  */
  if (VSCP_LEVEL1 == m_firmware_cfg.m_level) {
    ex.GUID[14]   = (m_firmware_cfg.m_nickname >> 8) & 0xff;
    ex.GUID[15]   = m_firmware_cfg.m_nickname & 0xff;
    ex.vscp_class = VSCP_CLASS1_CONTROL;
    ex.vscp_type  = VSCP_TYPE_CONTROL_TURNON;
    ex.sizeData   = 3;
    ex.data[0]    = idx;
    ex.data[1]    = psim1->m_reg_zone;
    ex.data[2]    = psim1->m_reg_subzone_S[idx];
  }
  else {
    ex.vscp_class = VSCP_CLASS1_CONTROL;
    ex.vscp_type  = VSCP_TYPE_CONTROL_TURNON;
    ex.sizeData   = 3;
    ex.data[0]    = idx;
    ex.data[1]    = psim1->m_reg_zone;
    ex.data[2]    = psim1->m_reg_subzone_S[idx];
  }

  return m_pClient->send(ex);
}

///////////////////////////////////////////////////////////////////////////////
// checkboxClick
//

int
btest::checkboxClick(int idx, bool checked)
{
  switch (m_nSimulation) {
    case 0:
    default:
      return checkboxClick_sim1(idx, checked);
      break;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// checkboxClick_sim1
//

int
btest::checkboxClick_sim1(int idx, bool checked)
{
  // Pointer to simulation storage must be valid
  if (nullptr == m_pSim) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if ((idx > 9) || (idx < 0)) {
    return VSCP_ERROR_PARAMETER;
  }

  simulation1_t* psim1 = (simulation1_t*)m_pSim;

  // Set value
  psim1->m_reg_value_C[idx] = (int)checked;

  // Send event
  vscpEventEx ex;

  memset(ex.data, 0, 512);
  ex.head = VSCP_PRIORITY_NORMAL;
  memcpy(ex.GUID, m_firmware_cfg.m_guid, 16);
  /*!
    If level I we use the GUID as a space for the nickname
  */
  if (VSCP_LEVEL1 == m_firmware_cfg.m_level) {
    ex.GUID[14]   = (m_firmware_cfg.m_nickname >> 8) & 0xff;
    ex.GUID[15]   = m_firmware_cfg.m_nickname & 0xff;
    ex.vscp_class = VSCP_CLASS1_INFORMATION;
    if (checked) {
      ex.vscp_type = VSCP_TYPE_INFORMATION_OPENED;
    }
    else {
      ex.vscp_type = VSCP_TYPE_INFORMATION_CLOSED;
    }
    ex.sizeData = 3;
    ex.data[0]  = idx;
    ex.data[1]  = psim1->m_reg_zone;
    ex.data[2]  = psim1->m_reg_subzone_C[idx];
  }
  else {
    ex.vscp_class = VSCP_CLASS1_CONTROL;
    ex.vscp_type  = VSCP_TYPE_CONTROL_TURNON;
    ex.sizeData   = 3;
    ex.data[0]    = idx;
    ex.data[1]    = psim1->m_reg_zone;
    ex.data[2]    = psim1->m_reg_subzone_C[idx];
  }

  return m_pClient->send(ex);
}

///////////////////////////////////////////////////////////////////////////////
// radioClick
//

int
btest::radioClick(int idx, bool checked)
{
  switch (m_nSimulation) {
    case 0:
    default:
      return radioClick_sim1(idx, checked);
      break;
  }

  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// radioClick_sim1
//

int
btest::radioClick_sim1(int idx, bool checked)
{
  // Pointer to simulation storage must be valid
  if (nullptr == m_pSim) {
    return VSCP_ERROR_INVALID_POINTER;
  }

  if ((idx > 9) || (idx < 0)) {
    return VSCP_ERROR_PARAMETER;
  }

  simulation1_t* psim1 = (simulation1_t*)m_pSim;

  // Set value
  psim1->m_reg_value_R[idx] = (int)checked;

  // Send event
  vscpEventEx ex;

  memset(ex.data, 0, 512);
  ex.head = VSCP_PRIORITY_NORMAL;
  memcpy(ex.GUID, m_firmware_cfg.m_guid, 16);
  /*!
    If level I we use the GUID as a space for the nickname
  */
  if (VSCP_LEVEL1 == m_firmware_cfg.m_level) {
    ex.GUID[14]   = (m_firmware_cfg.m_nickname >> 8) & 0xff;
    ex.GUID[15]   = m_firmware_cfg.m_nickname & 0xff;
    ex.vscp_class = VSCP_CLASS1_INFORMATION;
    if (checked) {
      ex.vscp_type = VSCP_TYPE_INFORMATION_ON;
    }
    else {
      ex.vscp_type = VSCP_TYPE_INFORMATION_OFF;
    }
    ex.sizeData = 3;
    ex.data[0]  = idx;
    ex.data[1]  = psim1->m_reg_zone;
    ex.data[2]  = psim1->m_reg_subzone_R[idx];
  }
  else {
    ex.vscp_class = VSCP_CLASS1_INFORMATION;
    if (checked) {
      ex.vscp_type = VSCP_TYPE_INFORMATION_ON;
    }
    else {
      ex.vscp_type = VSCP_TYPE_INFORMATION_OFF;
    }
    ex.sizeData = 3;
    ex.data[0]  = idx;
    ex.data[1]  = psim1->m_reg_zone;
    ex.data[2]  = psim1->m_reg_subzone_R[idx];
  }

  return m_pClient->send(ex);
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

    cguid guid(m_firmware_cfg.m_guid);
    m_pClient = new vscpClientSocketCan();
    if (nullptr == m_pClient) {
      spdlog::error("Unable to create client object {0}", m_interface.toStdString());
      return VSCP_ERROR_ERROR;
    }

    if (VSCP_ERROR_SUCCESS !=
        (rv = ((vscpClientSocketCan*)m_pClient)->init(m_configVector[0], guid.toString(), 0))) {
      delete (vscpClientSocketCan*)m_pClient;
      m_pClient = nullptr;
      spdlog::error("Unable to initialize socketcan client {0} rv={1}", m_interface.toStdString(), rv);
      return VSCP_ERROR_HARDWARE;
    }

    using namespace std::placeholders;
    auto cb = std::bind(&btest::receiveCallback, this, _1, _2);
    // lambda version for reference
    // auto cb = [this](auto a, auto b) { this->receiveCallback(a, b); };
    m_pClient->setCallbackEx(cb, this);

    if (VSCP_ERROR_SUCCESS != (rv = m_pClient->connect())) {
      delete (vscpClientSocketCan*)m_pClient;
      m_pClient = nullptr;
      spdlog::error("Unable to initialize socketcan client {0} rv={1}", m_interface.toStdString(), rv);
      return VSCP_ERROR_HARDWARE;
    }

    uint32_t timeout = vscp_getMsTimeStamp();
    while (!m_pClient->isConnected()) {
      spdlog::trace("Waiting for connect...");
      sleep(1);
      if ((vscp_getMsTimeStamp() - timeout) > m_timeoutConnect) {
        m_pClient->disconnect();
        spdlog::error("Timeout while waiting for connection of client {0}", m_interface.toStdString());
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

    if (VSCP_ERROR_SUCCESS != (rv = m_pClient->disconnect())) {
      delete (vscpClientSocketCan*)m_pClient;
      m_pClient = nullptr;
      spdlog::error("Unable to initialize socketcan client {0} rv={1}", m_interface.toStdString(), rv);
      return VSCP_ERROR_HARDWARE;
    }

    delete (vscpClientSocketCan*)m_pClient;
    m_pClient = nullptr;
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
  return m_pClient->send(*pex);
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

  // Return if we are supposed to end work
  if (!m_bRun) {
    return VSCP_ERROR_TIMEOUT;
  }

  // Return if error
  if (rv) {
    return rv;
  }

  pthread_mutex_lock(&m_mutexReceiveQueue);
  if (m_inqueue.size()) {
    pex = m_inqueue.dequeue();
    pthread_mutex_unlock(&m_mutexReceiveQueue);
    spdlog::trace("vscp_boot: getEventEx {0:X}:{1:X}", pex->vscp_class, pex->vscp_type);

    // If this is a proxy event then translate to standard event
    if ((pex->vscp_class >= 512) && ((pex->vscp_class < 1024))) {
      pex->vscp_class -= 512;                                // Standard level I class
      memcpy(pex->data, pex->data + 16, pex->sizeData - 16); // Remove proxy interface
    }
  }
  else {
    pthread_mutex_unlock(&m_mutexReceiveQueue);
    spdlog::trace("[vscpboot_getEventEx] No events to fetch");
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

    // Initialize hardware
    if (VSCP_ERROR_SUCCESS != (rv = vscpboot_init_hardware())) {
      spdlog::error("workerthread: Failed to initialize hardware. rv={}", rv);
      pbtest->m_bRun = false;
      return NULL;
    }

    // Initialize the firmware
    if (VSCP_ERROR_SUCCESS != (rv = vscp_frmw2_init(&pbtest->m_firmware_cfg))) {
      spdlog::error("workerthread: Failed to initialize firmware. rv={}", rv);
      pbtest->m_bRun = false;
      return NULL;
    }

    // Go to work

    vscpEventEx* pex;

    while (pbtest->m_bRun) {

      // get event from the input queue if there is one
      if (VSCP_ERROR_SUCCESS != (rv = pbtest->getEventEx(&pex))) {
        if ((VSCP_ERROR_TIMEOUT != rv) && (VSCP_ERROR_FIFO_EMPTY != rv)) {
          spdlog::error("workerthread: [getEventEx] failed in worktread. rv={0} {1}", rv, strerror(rv));
        }
      }

      // pex is NULL here if no event received
      if (NULL != pex) {
        spdlog::trace("workerthread: Read event ex: {0:X}:{1:X} size={2} Data: {3:X}",
                      pex->vscp_class,
                      pex->vscp_type,
                      pex->sizeData,
                      spdlog::to_hex(std::begin(pex->data), std::begin(pex->data) + pex->sizeData));
      }

      if (VSCP_ERROR_SUCCESS != (rv = vscp_frmw2_work(pex))) {
        spdlog::error("workerthread: [vscp_frmw2_work] Failed in worktread. rv={}", rv);
      }

      // Cleanup event
      delete pex;
      pex = NULL;
    }
  }

  return NULL;
}
