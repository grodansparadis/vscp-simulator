// btest.h
//
// This file is part of the VSCP (https://www.vscp.org)
//
// The MIT License (MIT)
//
// Copyright © 2000-2025 Ake Hedman, Grodans Paradis AB
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

#include <chrono>
#include <deque>
#include <fstream>
#include <thread>
#include <iostream>
#include <memory>
#include <set>
#include <stdio.h>
#include <string>

#include "btest.h"
#include <vscp.h>
#include <vscphelper.h>

#include <vscp-client-canal.h>
#include <vscp-client-level2.h>
#include <vscp-client-mqtt.h>
#include <vscp-client-multicast.h>
#include <vscp-client-tcp.h>
#include <vscp-client-udp.h>
#include <vscp-client-ws1.h>
#include <vscp-client-ws2.h>
#ifdef VSCPSIM_HAVE_SOCKETCAN
#include <vscp-client-socketcan.h>
#endif
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
  m_bThreadStarted = false;

  m_bRun = false;

  // Set to known state
  memset(&m_firmware_cfg, 0, sizeof(vscp_frmw2_firmware_config_t));
  m_firmware_cfg.ops                    = &s_firmwareOps;
  m_firmware_cfg.puserdata              = this;
  m_firmware_cfg.probe_timeout          = VSCP_FRMW2_UNASSIGNED; // Default timeouts
  m_firmware_cfg.probe_timeout_count    = VSCP_FRMW2_UNASSIGNED;
  m_firmware_cfg.nickname               = VSCP_ADDRESS_NEW_NODE; // Find a nickname (Level I)
  m_firmware_cfg.level                  = VSCP_LEVEL1;
  m_firmware_cfg.bInterestedInAllEvents = 1;

  m_bootflag = BOOTLOADER; // Start the bootloader

  // Set default connect timeout
  m_timeoutConnect = DEFAULT_CONNECT_TIMEOUT;

  // Bootloader defaults
  m_bootloader_cfg.vscpLevel  = VSCP_LEVEL2;
  m_bootloader_cfg.blockSize  = 0x800; // 2048
  m_bootloader_cfg.blockCount = 1000;

  m_bEndNormal = true; // App should end normally

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

  m_port = 0;

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
    spdlog::error("Failed to stop worker thread rv=%d", rv);
  }

  {
    std::lock_guard<std::mutex> lock(m_mutexReceiveQueue);
    m_inqueue.clear();
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
}

///////////////////////////////////////////////////////////////////////////////
// startWorkerThread *
//

int
btest::startWorkerThread(void)
{
  // Start the workerthread
  m_bRun = true; // Workerthread should run, run, run...
  if (pthread_create(&m_threadWork, NULL, workerThread, this)) {
    spdlog::critical("BTEST: Failed to start workerthread");
    return VSCP_ERROR_ERROR;
  }
  m_bThreadStarted = true;
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// stopWorkerThread
//

int
btest::stopWorkerThread(void)
{
  // Start the bootloader worker thread
  m_bRun = false; // Worker thread should end it's life
  m_cvReceiveQueue.notify_all();
  if (m_bThreadStarted) {
    pthread_join(m_threadWork, NULL);
    m_bThreadStarted = false;
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

    // Everything in the interface object is handed to the client
    m_interfaceConfig = j["interface"];

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

    // Connect timeout ("connect-timout" is the old spelling)
    const char* timeoutKey = j["interface"].contains("connect-timeout") ? "connect-timeout" : "connect-timout";
    if (j["interface"].contains(timeoutKey)) {
      if (j["interface"][timeoutKey].is_string()) {
        m_timeoutConnect = vscp_readStringValue(j["interface"][timeoutKey]);
      }
      else if (j["interface"][timeoutKey].is_number_integer()) {
        m_timeoutConnect = j["interface"][timeoutKey];
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
      strncpy((char*)m_firmware_cfg.deviceName, str.c_str(), 64);
    }

    if (jj.contains("level") && jj["level"].is_number_integer()) {
      switch ((int)jj["level"]) {

        case 2:
          m_firmware_cfg.level = VSCP_LEVEL2;
          break;

        default:
        case 1:
          m_firmware_cfg.level = VSCP_LEVEL1;
          break;
      }
    }

    if (jj.contains("guid")) {
      if (jj["guid"].is_string()) {
        std::string str = jj["guid"];
        cguid guid(str);
        memcpy(m_firmware_cfg.guid, guid.getGUID(), 16);
      }
      else if (jj["guid"].is_array() && (jj["guid"].size() <= 16)) {
        int idx = 0;
        cguid guid;
        for (auto& item : jj["guid"].items()) {
          guid.setAt(idx, (int)item.value());
          idx++;
        }
        memcpy(m_firmware_cfg.guid, guid.getGUID(), 16);
      }
    }

    if (jj.contains("mdfurl") && jj["mdfurl"].is_string()) {
      std::string str = jj["mdfurl"];
      strncpy((char*)m_firmware_cfg.mdfurl, str.c_str(), 32);
    }

    if (jj.contains("bUse16BitNickname") && jj["bUse16BitNickname"].is_boolean()) {
      m_firmware_cfg.bUse16BitNickname = jj["bUse16BitNickname"];
    }

    if (jj.contains("nickname")) {
      if (jj["nickname"].is_number_integer()) {
        m_firmware_cfg.nickname = jj["nickname"];
      }
      else if (jj["nickname"].is_string()) {
        m_firmware_cfg.nickname = vscp_readStringValue(jj["nickname"]);
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
        m_firmware_cfg.interval_heartbeat = jj["hertbeat-interval"];
      }
      else if (jj["hertbeat-interval"].is_string()) {
        m_firmware_cfg.interval_heartbeat = vscp_readStringValue(jj["hertbeat-interval"]);
      }
    }

    if (jj.contains("caps-interval")) {
      if (jj["caps-interval"].is_number_integer()) {
        m_firmware_cfg.interval_caps = jj["caps-interval"];
      }
      else if (jj["caps-interval"].is_string()) {
        m_firmware_cfg.interval_caps = vscp_readStringValue(jj["caps-interval"]);
      }
    }

    if (jj.contains("bEnableLogging") && jj["bEnableLogging"].is_boolean()) {
      m_firmware_cfg.bEnableLogging = jj["bEnableLogging"];
    }

    if (jj.contains("log-id")) {
      if (jj["log-id"].is_number_integer()) {
        m_firmware_cfg.log_id = jj["log-id"];
      }
      else if (jj["log-id"].is_string()) {
        m_firmware_cfg.log_id = vscp_readStringValue(jj["log-id"]);
      }
    }

    if (jj.contains("bEnableErrorReporting") && jj["bEnableErrorReporting"].is_boolean()) {
      m_firmware_cfg.bEnableErrorReporting = jj["bEnableErrorReporting"];
    }

    // if (jj.contains("bEnableErrorReporting") && jj["bEnableErrorReporting"].is_boolean()) {
    //   m_firmware_cfg.bSendHighEndServerProbe = jj["bEnableErrorReporting"];
    // }

    if (jj.contains("bHighEndServerResponse") && jj["bHighEndServerResponse"].is_boolean()) {
      m_firmware_cfg.bHighEndServerResponse = jj["bHighEndServerResponse"];
    }

    if (jj.contains("bEnableWriteProtectedLocations") && jj["bEnableWriteProtectedLocations"].is_boolean()) {
      m_firmware_cfg.bEnableWriteProtectedLocations = jj["bEnableWriteProtectedLocations"];
    }

    // User id is five bytes (registers 0x84 - 0x88). A number fills the
    // first four bytes, MSB first.
    if (jj.contains("user-id")) {
      memset(m_firmware_cfg.userId, 0, sizeof(m_firmware_cfg.userId));
      if (jj["user-id"].is_number_integer() || jj["user-id"].is_string()) {
        uint32_t userid = jj["user-id"].is_string() ? (uint32_t)vscp_readStringValue(jj["user-id"])
                                                    : (uint32_t)jj["user-id"].get<int64_t>();
        m_firmware_cfg.userId[0] = (userid >> 24) & 0xff;
        m_firmware_cfg.userId[1] = (userid >> 16) & 0xff;
        m_firmware_cfg.userId[2] = (userid >> 8) & 0xff;
        m_firmware_cfg.userId[3] = userid & 0xff;
      }
      else if (jj["user-id"].is_array() && (jj["user-id"].size() <= sizeof(m_firmware_cfg.userId))) {
        int idx = 0;
        for (auto& item : jj["user-id"].items()) {
          m_firmware_cfg.userId[idx] = (int)item.value();
          idx++;
        }
      }
    }

    if (jj.contains("manufactured-id")) {
      if (jj["manufactured-id"].is_number_integer()) {
        m_firmware_cfg.manufacturerId = (int)jj["manufactured-id"];
      }
      else if (jj["manufactured-id"].is_string()) {
        m_firmware_cfg.manufacturerId = vscp_readStringValue(jj["manufactured-id"]);
      }
      else if (jj["manufactured-id"].is_array() && (jj["manufactured-id"].size() <= 4)) {
        int idx = 0;
        uint8_t manufacturerid[4];
        for (auto& item : jj["manufactured-id"].items()) {
          manufacturerid[idx] = (int)item.value();
          idx++;
        }
        m_firmware_cfg.manufacturerId = construct_unsigned32(manufacturerid[0], manufacturerid[1], manufacturerid[2], manufacturerid[3]);
      }
    }

    if (jj.contains("manufacturer-sub-id")) {
      if (jj["manufacturer-sub-id"].is_number_integer()) {
        m_firmware_cfg.manufacturerSubId = (int)jj["manufacturer-sub-id"];
      }
      else if (jj["manufacturer-sub-id"].is_string()) {
        m_firmware_cfg.manufacturerSubId = vscp_readStringValue(jj["manufacturer-sub-id"]);
      }
      else if (jj["manufacturer-sub-id"].is_array() && (jj["manufacturer-sub-id"].size() <= 4)) {
        int idx = 0;
        uint8_t manufacturersubid[4];
        for (auto& item : jj["manufacturer-sub-id"].items()) {
          manufacturersubid[idx] = (int)item.value();
          idx++;
        }
        m_firmware_cfg.manufacturerSubId = construct_unsigned32(manufacturersubid[0], manufacturersubid[1], manufacturersubid[2], manufacturersubid[3]);
      }
    }

    if (jj.contains("firmware-version") && jj["firmware-version"].is_array() && (jj["firmware-version"].size() <= 3)) {
      int idx = 0;
      for (auto& item : jj["firmware-version"].items()) {
        switch (idx) {
          case 0:
            m_firmware_cfg.firmware_major_version = (int)item.value();
            break;

          case 1:
            m_firmware_cfg.firmware_minor_version = (int)item.value();
            break;

          case 2:
            m_firmware_cfg.firmware_sub_minor_version = (int)item.value();
            break;
        }
        idx++;
      }
    }

    if (jj.contains("bootloader-algorithm")) {
      if (jj["bootloader-algorithm"].is_number_integer()) {
        m_firmware_cfg.bootloader_algorithm = jj["bootloader-algorithm"];
      }
      else if (jj["bootloader-algorithm"].is_string()) {
        m_firmware_cfg.bootloader_algorithm = vscp_readStringValue(jj["bootloader-algorithm"]);
      }
    }

    if (jj.contains("standard-device-family-code")) {
      if (jj["standard-device-family-code"].is_number_integer()) {
        m_firmware_cfg.standard_device_family = (int)jj["standard-device-family-code"];
      }
      else if (jj["standard-device-family-code"].is_string()) {
        m_firmware_cfg.standard_device_family = vscp_readStringValue(jj["standard-device-family-code"]);
      }
      else if (jj["standard-device-family-code"].is_array() && (jj["standard-device-family-code"].size() <= 4)) {
        int idx = 0;
        uint8_t val[4];
        for (auto& item : jj["standard-device-family-code"].items()) {
          val[idx] = (int)item.value();
          idx++;
        }
        m_firmware_cfg.standard_device_family = construct_unsigned32(val[0], val[1], val[2], val[3]);
      }
    }

    if (jj.contains("standard-device-type-code")) {
      if (jj["standard-device-type-code"].is_number_integer()) {
        m_firmware_cfg.standard_device_type = (int)jj["standard-device-type-code"];
      }
      else if (jj["standard-device-type-code"].is_string()) {
        m_firmware_cfg.standard_device_type = vscp_readStringValue(jj["standard-device-type-code"]);
      }
      else if (jj["standard-device-type-code"].is_array() && (jj["standard-device-type-code"].size() <= 4)) {
        int idx = 0;
        uint8_t val[4];
        for (auto& item : jj["standard-device-type-code"].items()) {
          val[idx] = (int)item.value();
          idx++;
        }
        m_firmware_cfg.standard_device_type = construct_unsigned32(val[0], val[1], val[2], val[3]);
      }
    }

    if (jj.contains("firmware-device-code")) {
      if (jj["firmware-device-code"].is_number_integer()) {
        m_firmware_cfg.firmware_device_code = (int)jj["firmware-device-code"];
      }
      else if (jj["firmware-device-code"].is_string()) {
        m_firmware_cfg.firmware_device_code = vscp_readStringValue(jj["firmware-device-code"]);
      }
      else if (jj["firmware-device-code"].is_array() && (jj["firmware-device-code"].size() <= 2)) {
        int idx = 0;
        uint8_t val[2];
        for (auto& item : jj["firmware-device-code"].items()) {
          val[idx] = (int)item.value();
          idx++;
        }
        m_firmware_cfg.firmware_device_code = construct_unsigned16(val[0], val[1]);
      }
    }

    if (jj.contains("ip-addr")) {
      if (jj["ip-addr"].is_number_integer()) {
        // Numerical ipv4 address
        uint32_t ipaddr            = (int)jj["ip-addr"];
        m_firmware_cfg.ipaddr[0] = *((uint8_t*)&ipaddr);
        m_firmware_cfg.ipaddr[1] = *((uint8_t*)&ipaddr + 1);
        m_firmware_cfg.ipaddr[2] = *((uint8_t*)&ipaddr + 2);
        m_firmware_cfg.ipaddr[3] = *((uint8_t*)&ipaddr + 3);
      }
      else if (jj["ip-addr"].is_string()) {
        std::string str = jj["ip-addr"];
        std::deque<std::string> parts;
        // ipv4 address
        vscp_split(parts, str, ".");
        if (parts.size()) {
          int idx = 0;
          for (const auto& x : parts) {
            m_firmware_cfg.ipaddr[idx] = atoi(x.c_str());
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
              m_firmware_cfg.ipaddr[idx]     = (val >> 8) & 0xff;
              m_firmware_cfg.ipaddr[idx + 1] = val & 0xff;
              idx += 2;
            }
          }
        }
      }
      else if (jj["ip-addr"].is_array() && (jj["ip-addr"].size() <= 16)) {
        int idx = 0;
        for (auto& item : jj["ip-addr"].items()) {
          m_firmware_cfg.ipaddr[idx] = (int)item.value();
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
        m_firmware_cfg.nDmRows = jj["dm-rows"];
      }
      else if (jj["dm-rows"].is_string()) {
        m_firmware_cfg.nDmRows = vscp_readStringValue(jj["dm-rows"]);
      }
    }

    if (VSCP_LEVEL1 == m_firmware_cfg.level) {
      m_firmware_cfg.sizeDmRow = 8;
    }
    else {
      if (jj.contains("dm-size")) {
        if (jj["dm-size"].is_number_integer()) {
          m_firmware_cfg.sizeDmRow = jj["dm-size"];
        }
        else if (jj["dm-size"].is_string()) {
          m_firmware_cfg.sizeDmRow = vscp_readStringValue(jj["dm-size"]);
        }
      }
    }

    if (jj.contains("dm-offset")) {
      if (jj["dm-offset"].is_number_integer()) {
        m_firmware_cfg.regOffsetDm = jj["dm-offset"];
      }
      else if (jj["dm-offset"].is_string()) {
        m_firmware_cfg.regOffsetDm = vscp_readStringValue(jj["dm-offset"]);
      }
    }

    if (jj.contains("dm-page-start")) {
      if (jj["dm-page-start"].is_number_integer()) {
        m_firmware_cfg.pageDm = jj["dm-page-start"];
      }
      else if (jj["dm-page-start"].is_string()) {
        m_firmware_cfg.pageDm = vscp_readStringValue(jj["dm-page-start"]);
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
// Called by the client (from its own thread) for each received event.
//

void
btest::receiveCallback(vscpEventEx& ex, void* /*pobj*/)
{
  // Our own events can come back, for example UDP broadcasts. The node
  // must not react on them.
  if (VSCP_LEVEL1 == m_firmware_cfg.level) {
    if ((0 == memcmp(ex.GUID, m_firmware_cfg.guid, 14)) &&
        (ex.GUID[14] == ((m_firmware_cfg.nickname >> 8) & 0xff)) &&
        (ex.GUID[15] == (m_firmware_cfg.nickname & 0xff)) && (VSCP_ADDRESS_NEW_NODE != m_firmware_cfg.nickname)) {
      return;
    }
  }
  else if (0 == memcmp(ex.GUID, m_firmware_cfg.guid, 16)) {
    return;
  }

  spdlog::trace("[receiveCallback] Received {0}:{1} size={2}", ex.vscp_class, ex.vscp_type, ex.sizeData);

  {
    std::lock_guard<std::mutex> lock(m_mutexReceiveQueue);
    if (m_inqueue.size() >= MAX_INQUEUE) {
      spdlog::warn("Receive queue full, oldest event dropped");
      m_inqueue.pop_front();
    }
    m_inqueue.push_back(ex);
  }
  m_cvReceiveQueue.notify_one();
}

///////////////////////////////////////////////////////////////////////////////
// getEventEx
//

int
btest::getEventEx(vscpEventEx& ex, uint32_t timeout)
{
  std::unique_lock<std::mutex> lock(m_mutexReceiveQueue);
  if (!m_cvReceiveQueue.wait_for(lock, std::chrono::milliseconds(timeout), [this] {
        return !m_inqueue.empty() || !m_bRun;
      }) ||
      m_inqueue.empty()) {
    return VSCP_ERROR_FIFO_EMPTY;
  }

  ex = m_inqueue.front();
  m_inqueue.pop_front();
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// sendEvent
//

int
btest::sendEvent(vscpEventEx& ex)
{
  if (nullptr == m_pClient) {
    return VSCP_ERROR_NOT_CONNECTED;
  }

  // Time stamp events that have no time
  if ((0 == ex.year) && (0 == ex.timestamp)) {
    vscp_setEventExDateTimeBlockToNow(&ex);
  }

  int rv = m_pClient->send(ex);
  if (VSCP_ERROR_SUCCESS != rv) {
    spdlog::warn("Failed to send event {0}:{1} rv={2}", ex.vscp_class, ex.vscp_type, rv);
  }
  return rv;
}

///////////////////////////////////////////////////////////////////////////////
// isSupportedInterface
//

bool
btest::isSupportedInterface(const QString& type)
{
#ifdef VSCPSIM_HAVE_SOCKETCAN
  if ("socketcan" == type) {
    return true;
  }
#endif
  return ("tcpip" == type) || ("tcp" == type) || ("udp" == type) || ("multicast" == type) || ("ws1" == type) ||
         ("ws2" == type) || ("mqtt" == type) || ("canal" == type) || ("level2" == type);
}

///////////////////////////////////////////////////////////////////////////////
// getClientConfig
//
// The configuration file "interface" object is used as is (except the keys
// only the simulator uses). Settings that are missing are taken from the
// command line options, or get a default for the interface type.
//

json
btest::getClientConfig(void)
{
  json cfg = m_interfaceConfig.is_object() ? m_interfaceConfig : json::object();
  cfg.erase("type");
  cfg.erase("connect-timeout");
  cfg.erase("connect-timout");

  const std::string host = m_host.trimmed().toStdString();
  const std::string user = m_user.toStdString();
  const std::string pass = m_password.toStdString();

  auto setDefault = [&cfg](const char* key, const json& value) {
    if (!cfg.contains(key)) {
      cfg[key] = value;
    }
  };

  // IPv6 addresses need brackets in a URL
  auto urlHost = [](const std::string& h) {
    return ((std::string::npos != h.find(':')) && ('[' != h[0])) ? ("[" + h + "]") : h;
  };

  if (("tcpip" == m_interface) || ("tcp" == m_interface)) {
    setDefault("host",
               "tcp://" + urlHost(host.empty() ? "localhost" : host) + ":" +
                 std::to_string(m_port ? m_port : VSCP_DEFAULT_TCP_PORT));
    setDefault("user", user.empty() ? "admin" : user);
    setDefault("password", pass.empty() ? "secret" : pass);
    setDefault("bfull-l2", VSCP_LEVEL2 == m_firmware_cfg.level);
  }
  else if ("udp" == m_interface) {
    // Default is broadcast on the VSCP UDP port
    if (!host.empty() || m_port) {
      setDefault("ip",
                 "udp://" + urlHost(host.empty() ? "255.255.255.255" : host) + ":" +
                   std::to_string(m_port ? m_port : VSCP_DEFAULT_UDP_PORT));
    }
  }
  else if ("multicast" == m_interface) {
    // Default is the VSCP multicast group 224.0.23.158:9598
    if (!host.empty() || m_port) {
      setDefault("ip",
                 "udp://" + urlHost(host.empty() ? "224.0.23.158" : host) + ":" +
                   std::to_string(m_port ? m_port : 9598)); // Same default as the client
    }
  }
  else if (("ws1" == m_interface) || ("ws2" == m_interface)) {
    setDefault("url",
               "ws://" + urlHost(host.empty() ? "localhost" : host) + ":" + std::to_string(m_port ? m_port : 8884) +
                 "/" + m_interface.toStdString());
    setDefault("user", user.empty() ? "admin" : user);
    setDefault("password", pass.empty() ? "secret" : pass);
  }
  else if ("mqtt" == m_interface) {
    setDefault("host", host.empty() ? "localhost" : host);
    setDefault("port", m_port ? m_port : 1883);
    if (!user.empty()) {
      setDefault("user", user);
      setDefault("password", pass);
    }
  }
  else if ("canal" == m_interface) {
    // Without a "path" key the configuration string is "driver path;CANAL
    // configuration" (command line --config or "config" in the file). The
    // CANAL configuration itself uses ';', so the rest is joined again.
    if (!cfg.contains("path") && m_configVector.size()) {
      cfg["path"] = m_configVector[0];
      std::string drvcfg;
      for (size_t i = 1; i < m_configVector.size(); i++) {
        drvcfg += (i > 1 ? ";" : "") + m_configVector[i];
      }
      cfg["config"] = drvcfg;
    }
    setDefault("config", "");
    setDefault("flags", 0);
    setDefault("name", "vscpsim");
  }
  else if ("level2" == m_interface) {
    // Without a "path-driver" key the configuration string is "driver
    // path;driver configuration file". "config" is then not an embedded
    // driver configuration, so it is removed.
    if (!cfg.contains("path-driver") && m_configVector.size()) {
      cfg["path-driver"] = m_configVector[0];
      cfg.erase("config");
      if (m_configVector.size() > 1) {
        cfg["path-config"] = m_configVector[1];
      }
    }
    setDefault("name", "vscpsim");
  }
  else if ("socketcan" == m_interface) {
    if (!cfg.contains("device")) {
      cfg["device"] = m_configVector.size() ? m_configVector[0] : std::string("vcan0");
    }
  }

  return cfg;
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
      spdlog::error("Invalid simulation set. Will use simulation 1");
      break;

    case 1: {

      m_pSim = new simulation1_t;
      memset(m_pSim, 0, sizeof(simulation1_t));
      m_firmware_cfg.pEventsOfInterest              = nullptr;
      ((simulation1_t*)m_pSim)->m_background_color[0] = 0xc0;
      ((simulation1_t*)m_pSim)->m_background_color[1] = 0xc0;
      ((simulation1_t*)m_pSim)->m_background_color[2] = 0xc0;

      // Page 0
      for (int i = 0; i <= 86; i++) {
        m_regmap[i] = nullptr; // No table item yet (inserted by main window)
      }

      // Standard registers
      for (int i = 128; i <= 255; i++) {
        m_regmap[i] = nullptr; // No table item yet (inserted by main window)
      }

      // page 1
      for (int i = 0; i <= 3; i++) {
        m_regmap[(1 << 16) + i] = nullptr; // No table item yet (inserted by main window)
      }

      // page 2
      for (int i = 0; i <= 3; i++) {
        m_regmap[(2 << 16) + i] = nullptr; // No table item yet (inserted by main window)
      }

      emit updateRegister(82, 0, 0xff);
      emit updateRegister(83, 0, 0xff);
      emit updateRegister(84, 0, 0xff);

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

  if (idx > 9) {
    return VSCP_ERROR_PARAMETER;
  }

  simulation1_t* psim1 = (simulation1_t*)m_pSim;

  // Save value
  psim1->m_reg_value_slider[idx] = value;

  /*
    If nickname is not set we don't allow any active
    functionality until it is
  */
  if (VSCP_ADDRESS_NEW_NODE == m_firmware_cfg.nickname) {
    return VSCP_ERROR_INIT_MISSING;
  }

  // Send event
  vscpEventEx ex;

  memset(&ex, 0, sizeof(ex));
  ex.head = VSCP_PRIORITY_NORMAL;
  memcpy(ex.GUID, m_firmware_cfg.guid, 16);
  /*!
    If level I we use the GUID as a space for the nickname
  */
  if (VSCP_LEVEL1 == m_firmware_cfg.level) {
    ex.GUID[14]   = (m_firmware_cfg.nickname >> 8) & 0xff;
    ex.GUID[15]   = m_firmware_cfg.nickname & 0xff;
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

  return sendEvent(ex);
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
btest::getMdfUrl(uint8_t* const /*purl*/)
{
  // uint8_t buf[10];
  // memcpy(purl, buf, 10);
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// receivedSegCtrlHeartBeat
//

int
btest::receivedSegCtrlHeartBeat(uint16_t /*segcrc*/, uint32_t /*tm*/)
{
  return VSCP_ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
// reportEventsOfInterest
//
// The simulated node is interested in all events (bInterestedInAllEvents),
// so there is no list to report.
//

int
btest::reportEventsOfInterest(void)
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
      value = m_firmware_cfg.alarm_status;
      break;

    case VSCP_STD_REGISTER_MAJOR_VERSION:
      value = m_firmware_cfg.vscp_major_version;
      break;

    case VSCP_STD_REGISTER_MINOR_VERSION:
      value = m_firmware_cfg.vscp_minor_version;
      break;

    case VSCP_STD_REGISTER_ERROR_COUNTER:
      value = m_firmware_cfg.errorCounter;
      break;

    case VSCP_STD_REGISTER_USER_ID:
    case VSCP_STD_REGISTER_USER_ID + 1:
    case VSCP_STD_REGISTER_USER_ID + 2:
    case VSCP_STD_REGISTER_USER_ID + 3:
    case VSCP_STD_REGISTER_USER_ID + 4:
      value = m_firmware_cfg.userId[reg - VSCP_STD_REGISTER_USER_ID];
      break;

    case VSCP_STD_REGISTER_USER_MANDEV_ID:
      value = (m_firmware_cfg.manufacturerId >> 24) & 0xff;
      break;
    case VSCP_STD_REGISTER_USER_MANDEV_ID + 1:
      value = (m_firmware_cfg.manufacturerId >> 16) & 0xff;
      break;
    case VSCP_STD_REGISTER_USER_MANDEV_ID + 2:
      value = (m_firmware_cfg.manufacturerId >> 8) & 0xff;
      break;
    case VSCP_STD_REGISTER_USER_MANDEV_ID + 3:
      value = m_firmware_cfg.manufacturerId & 0xff;
      break;

    case VSCP_STD_REGISTER_USER_MANSUBDEV_ID:
      value = (m_firmware_cfg.manufacturerSubId >> 24) & 0xff;
      break;
    case VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 1:
      value = (m_firmware_cfg.manufacturerSubId >> 16) & 0xff;
      break;
    case VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 2:
      value = (m_firmware_cfg.manufacturerSubId >> 8) & 0xff;
      break;
    case VSCP_STD_REGISTER_USER_MANSUBDEV_ID + 3:
      value = m_firmware_cfg.manufacturerSubId & 0xff;
      break;

      // See below
      // case VSCP_STD_REGISTER_NICKNAME_ID_LSB:
      //  break;

    case VSCP_STD_REGISTER_PAGE_SELECT_MSB:
      value = (m_firmware_cfg.page_select >> 16) & 0xff;
      break;

    case VSCP_STD_REGISTER_PAGE_SELECT_LSB:
      value = m_firmware_cfg.page_select & 0xff;
      break;

    case VSCP_STD_REGISTER_FIRMWARE_MAJOR:
      value = m_firmware_cfg.firmware_major_version;
      break;

    case VSCP_STD_REGISTER_FIRMWARE_MINOR:
      value = m_firmware_cfg.firmware_minor_version;
      break;

    case VSCP_STD_REGISTER_FIRMWARE_SUBMINOR:
      value = m_firmware_cfg.firmware_sub_minor_version;
      break;

    case VSCP_STD_REGISTER_BOOT_LOADER:
      value = m_firmware_cfg.bootloader_algorithm;
      break;

    case VSCP_STD_REGISTER_BUFFER_SIZE:
      value = 0;
      break;

    case VSCP_STD_REGISTER_PAGES_COUNT:
      value = 0;
      break;

    // Device family and type are four bytes, MSB first
    case VSCP_STD_REGISTER_DEVICE_FAMILY:
    case VSCP_STD_REGISTER_DEVICE_FAMILY + 1:
    case VSCP_STD_REGISTER_DEVICE_FAMILY + 2:
    case VSCP_STD_REGISTER_DEVICE_FAMILY + 3:
      value = (m_firmware_cfg.standard_device_family >> (8 * (3 - (reg - VSCP_STD_REGISTER_DEVICE_FAMILY)))) & 0xff;
      break;

    case VSCP_STD_REGISTER_DEVICE_TYPE:
    case VSCP_STD_REGISTER_DEVICE_TYPE + 1:
    case VSCP_STD_REGISTER_DEVICE_TYPE + 2:
    case VSCP_STD_REGISTER_DEVICE_TYPE + 3:
      value = (m_firmware_cfg.standard_device_type >> (8 * (3 - (reg - VSCP_STD_REGISTER_DEVICE_TYPE)))) & 0xff;
      break;

    case VSCP_STD_REGISTER_NODE_RESET:
      value = 0;
      break;

    case VSCP_STD_REGISTER_FIRMWARE_CODE_MSB:
      value = (m_firmware_cfg.firmware_device_code >> 8) & 0xff;
      break;

    case VSCP_STD_REGISTER_FIRMWARE_CODE_LSB:
      value = m_firmware_cfg.firmware_device_code & 0xff;
      break;

    case VSCP_STD_REGISTER_NICKNAME_ID_LSB:
      value = m_firmware_cfg.nickname & 0xff;
      break;

    case VSCP_STD_REGISTER_NICKNAME_ID_MSB:
      value = (m_firmware_cfg.nickname >> 8) & 0xff;
      break;

    default:
      if ((reg >= VSCP_STD_REGISTER_GUID) && (reg < (VSCP_STD_REGISTER_GUID + 16))) {
        value = m_firmware_cfg.guid[reg - VSCP_STD_REGISTER_GUID];
      }
      // Is always <= 0xff so no need to test on that
      else if ((reg >= VSCP_STD_REGISTER_DEVICE_URL)) {
        value = m_firmware_cfg.mdfurl[reg - VSCP_STD_REGISTER_DEVICE_URL];
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

  memset(&ex, 0, sizeof(ex));
  ex.head = VSCP_PRIORITY_NORMAL;
  memcpy(ex.GUID, m_firmware_cfg.guid, 16);
  /*!
    If level I we use the GUID as a space for the nickname
  */
  if (VSCP_LEVEL1 == m_firmware_cfg.level) {
    ex.GUID[14]   = (m_firmware_cfg.nickname >> 8) & 0xff;
    ex.GUID[15]   = m_firmware_cfg.nickname & 0xff;
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

  return sendEvent(ex);
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

  memset(&ex, 0, sizeof(ex));
  ex.head = VSCP_PRIORITY_NORMAL;
  memcpy(ex.GUID, m_firmware_cfg.guid, 16);
  /*!
    If level I we use the GUID as a space for the nickname
  */
  if (VSCP_LEVEL1 == m_firmware_cfg.level) {
    ex.GUID[14]   = (m_firmware_cfg.nickname >> 8) & 0xff;
    ex.GUID[15]   = m_firmware_cfg.nickname & 0xff;
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

  return sendEvent(ex);
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

  memset(&ex, 0, sizeof(ex));
  ex.head = VSCP_PRIORITY_NORMAL;
  memcpy(ex.GUID, m_firmware_cfg.guid, 16);
  /*!
    If level I we use the GUID as a space for the nickname
  */
  if (VSCP_LEVEL1 == m_firmware_cfg.level) {
    ex.GUID[14]   = (m_firmware_cfg.nickname >> 8) & 0xff;
    ex.GUID[15]   = m_firmware_cfg.nickname & 0xff;
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

  return sendEvent(ex);
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
  // btest* pbtest = (btest*)QApplication::instance();
}

/*!
  Initialize hardware. This should be the first method called.
  The communication interface should be opened here and the system
  should be ready to receive firmware code events when done.
*/
int
btest::vscpboot_init_hardware(void)
{
  int rv;

  // Already open (the bootloader and the firmware both call this)
  if (nullptr != m_pClient) {
    return VSCP_ERROR_SUCCESS;
  }

  if (!isSupportedInterface(m_interface)) {
    spdlog::error("Interface type '{0}' is not supported. Use canal, level2, socketcan (Linux), tcpip, udp, multicast, ws1, ws2 or mqtt.",
                  m_interface.toStdString());
    return VSCP_ERROR_NOT_SUPPORTED;
  }

  if (("tcpip" == m_interface) || ("tcp" == m_interface)) {
    m_pClient = new vscpClientTcp();
  }
  else if ("udp" == m_interface) {
    m_pClient = new vscpClientUdp();
  }
  else if ("multicast" == m_interface) {
    m_pClient = new vscpClientMulticast();
  }
  else if ("ws1" == m_interface) {
    m_pClient = new vscpClientWs1();
  }
  else if ("ws2" == m_interface) {
    m_pClient = new vscpClientWs2();
  }
  else if ("mqtt" == m_interface) {
    m_pClient = new vscpClientMqtt();
  }
  else if ("canal" == m_interface) {
    // Level I CANAL driver (shared library)
    m_pClient = new vscpClientCanal();
  }
  else if ("level2" == m_interface) {
    // VSCP Level II driver (shared library)
    m_pClient = new vscpClientLevel2();
  }
#ifdef VSCPSIM_HAVE_SOCKETCAN
  else if ("socketcan" == m_interface) {
    m_pClient = new vscpClientSocketCan();
  }
#endif

  json cfg = getClientConfig();
  if (!m_pClient->initFromJson(cfg.dump())) {
    spdlog::error("Invalid configuration for interface {0}", m_interface.toStdString());
    delete m_pClient;
    m_pClient = nullptr;
    return VSCP_ERROR_PARAMETER;
  }

#ifdef VSCPSIM_HAVE_SOCKETCAN
  // The CAN client builds a GUID for received events from its interface GUID
  if ("socketcan" == m_interface) {
    cguid guid(m_firmware_cfg.guid);
    if (VSCP_ERROR_SUCCESS != (rv = ((vscpClientSocketCan*)m_pClient)
                                      ->init(cfg["device"].get<std::string>(), guid.toString(),
                                             cfg.value("flags", (unsigned long)0)))) {
      spdlog::error("Unable to initialize socketcan client rv={0}", rv);
      delete m_pClient;
      m_pClient = nullptr;
      return VSCP_ERROR_HARDWARE;
    }
  }
#endif

  // Received events go to the input queue
  using namespace std::placeholders;
  auto cb = std::bind(&btest::receiveCallback, this, _1, _2);
  m_pClient->setCallbackEx(cb, this);

  spdlog::info("Connecting to {0} interface", m_interface.toStdString());
  if (VSCP_ERROR_SUCCESS != (rv = m_pClient->connect())) {
    spdlog::error("Unable to connect the {0} interface rv={1}", m_interface.toStdString(), rv);
    delete m_pClient;
    m_pClient = nullptr;
    return VSCP_ERROR_HARDWARE;
  }

  // Some clients (MQTT) connect in the background
  uint32_t start = vscp_getMsTimeStamp();
  while (!m_pClient->isConnected()) {
    if (!m_bRun || ((vscp_getMsTimeStamp() - start) > m_timeoutConnect)) {
      spdlog::error("Timeout while waiting for the {0} interface to connect", m_interface.toStdString());
      m_pClient->disconnect();
      delete m_pClient;
      m_pClient = nullptr;
      return VSCP_ERROR_TIMEOUT;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  spdlog::info("Interface {0} connected", m_interface.toStdString());
  return VSCP_ERROR_SUCCESS;
}

/*!
  Free any hardware resources that needs to be free'd
*/
int
btest::vscpboot_release_hardware()
{
  int rv = VSCP_ERROR_SUCCESS;

  // If already released there is nothing to do
  if (nullptr == m_pClient) {
    return VSCP_ERROR_SUCCESS;
  }

  if (VSCP_ERROR_SUCCESS != (rv = m_pClient->disconnect())) {
    spdlog::error("Unable to disconnect the {0} interface rv={1}", m_interface.toStdString(), rv);
  }

  delete m_pClient;
  m_pClient = nullptr;

  std::lock_guard<std::mutex> lock(m_mutexReceiveQueue);
  m_inqueue.clear();

  return rv;
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
  Reboot the board. This method should never return
  on real hardware.
*/
void
btest::vscpboot_reboot(void)
{
  // // We restart the worker thread
  // if (VSCP_ERROR_SUCCESS != (rv = stopWorkerThread())) {
  //   spdlog::error("Failed to stop worker thread rv=%d", rv);
  // }

  // // Go into bootloader mode
  // vscpboot_setBootFlag(BOOTLOADER);

  // m_bRun = true;
  // if (VSCP_ERROR_SUCCESS != (rv = startWorkerThread())) {
  //   spdlog::error("Failed to start worker thread rv=%d", rv);
  // }

  // We should restart the main window after termination
  m_bEndNormal = false;

  // Stop the firmware/bootloader loop
  m_bRun = false;

  // This is called from the worker thread. Widgets may only be touched
  // from the GUI thread, so close the main window from there.
  QMetaObject::invokeMethod(
    this,
    []() {
      foreach (QWidget* w, qApp->topLevelWidgets()) {
        if (QMainWindow* mainWin = qobject_cast<QMainWindow*>(w)) {
          mainWin->close();
        }
      }
    },
    Qt::QueuedConnection);
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
  return m_firmware_cfg.guid;
}

/*!
  Check if memtype is valid. type == 0 is always valid
  @return non zero if valid
*/
int
btest::vscpboot_isMemTypeValid(uint8_t /*type*/)
{
  return TRUE;
}

/*!
  Check if membank is valid. type == 0 is always valid
  @return non zero if valid
*/
int
btest::vscpboot_isMemBankValid(uint8_t /*bank*/)
{
  return TRUE;
}

/*!
  Program a block
  @param pblock Pointer to the block to program
  @param type Memory type
  @param bank Memory bank to program
  @return VSCP_ERROR_SUCCESS on success and else errocode
*/
int
btest::vscpboot_programBlock(const uint8_t* /*pblock*/, uint8_t /*type*/, uint8_t /*bank*/)
{
  return VSCP_ERROR_SUCCESS;
}

/*!
  The CRC for the loaded data is calculated here. This is the CRC
  over all blocks programmed calculated with CRC-CCITT. For a successful
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
  if (nullptr == pex) {
    return VSCP_ERROR_INVALID_POINTER;
  }
  return sendEvent(*pex);
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
btest::vscpboot_getEventEx(vscpEventEx* /*pex*/)
{
  //   int rv;

  //   printf("vscpboot_getEventEx: Size %lu\n", (unsigned long)m_inqueue.size());

  // RCVLOOP:
  //   if ((-1 == (rv = vscp_sem_wait(&m_semReceiveQueue, 10))) &&
  //       errno == ETIMEDOUT) {
  //     // If app. should run continue blocking operation
  //     if (m_bRun) {
  //       goto RCVLOOP;
  //     }
  //   }

  //   // Return if we are supposed to end work
  //   if (!m_bRun) {
  //     return VSCP_ERROR_TIMEOUT;
  //   }

  //   // Return if error
  //   if (rv) {
  //     return rv;
  //   }

  //   pthread_mutex_lock(&m_mutexReceiveQueue);
  //   if (m_inqueue.size()) {

  //     pex = m_inqueue.dequeue();
  //     pthread_mutex_unlock(&m_mutexReceiveQueue);
  //     spdlog::trace("vscpboot_getEventEx: {0:X}:{1:X}", pex->vscp_class, pex->vscp_type);

  //     // If this is a proxy event then translate to standard event
  //     if ((pex->vscp_class >= 512) && (pex->vscp_class < 1024)) {
  //       pex->vscp_class -= 512;                                // Standard level I class
  //       memcpy(pex->data, pex->data + 16, pex->sizeData - 16); // Remove proxy interface
  //     }
  //   }
  //   else {
  //     pthread_mutex_unlock(&m_mutexReceiveQueue);
  //     spdlog::trace("[vscpboot_getEventEx] No events to fetch");
  //     return VSCP_ERROR_FIFO_EMPTY;
  //   }

  return VSCP_ERROR_SUCCESS;
}

/*
  Delete VSCP event
  @param pex Pointer to VSCP event ex.
*/
// void
// btest::vscpboot_deleteEventEx(vscpEventEx** pex)
// {
//   // if (nullptr != pex) {
//   //   delete *pex;
//   //   pex = nullptr;
//   // }
// }

//////////////////////////////////////////////////////////////////////
//                     Workerthread -
//////////////////////////////////////////////////////////////////////

void*
workerThread(void* pData)
{
  int rv;
  // fd_set rdfs;
  // struct timeval tv;

  btest* pbtest = (btest*)pData;
  if (NULL == pbtest) {
    spdlog::error("btest: No object data object supplied for worker thread");
    return NULL;
  }

THREAD_START:

  /*!
    Run in bootloader mode if bootflag is non-zero
    otherwise run the simulated firmware
  */
  if (pbtest->vscpboot_getBootFlag()) {

    emit pbtest->updateWindowsTitle(0xff);
    spdlog::info("Starting simulation software in bootloader mode");

    /* Start the bootloader
      !!! Initialization of hardware is done in the loader!!!
    */
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
      goto THREAD_END;
    }

    // Go to work
    while (pbtest->m_bRun) {

      // Wait a short while for an event. The firmware is called also
      // when nothing has been received so it can send heartbeats etc.
      vscpEventEx ex;
      vscpEvent ev;
      memset(&ev, 0, sizeof(ev));
      vscpEvent* pev = nullptr;

      if (VSCP_ERROR_SUCCESS == pbtest->getEventEx(ex, 10)) {
        if (vscp_convertEventExToEvent(&ev, &ex)) {
          pev = &ev;
          spdlog::trace("workerthread: Event {0:X}:{1:X} size={2}", ev.vscp_class, ev.vscp_type, ev.sizeData);
        }
      }

      rv = vscp_frmw2_work(&pbtest->m_firmware_cfg, pev);

      // The firmware does not keep the event
      delete[] ev.pdata;

      if (VSCP_ERROR_SUCCESS != rv) {
        /*
          If we get VSCP_ERROR_STOPPED we should stop the worker thread
          and release hardware resources. This is used only by the
          simulator. On real hardware the bootloader will be jumped to
          when asked to go in that direction.
        */
        if (VSCP_ERROR_STOPPED == rv) {
          goto THREAD_START;
        }
        spdlog::trace("workerthread: [vscp_frmw2_work] rv={}", rv);
      }

    } // while
  } // Firmware mode

THREAD_END:

  // Release hardware
  if (VSCP_ERROR_SUCCESS != (rv = vscpboot_release_hardware())) {
    spdlog::error("workerthread: Failed to release hardware. rv={}", rv);
    pbtest->m_bRun = false;
    return NULL;
  }

  return NULL;
}
