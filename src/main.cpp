// main.cpp
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

#include "btest.h"
#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>
#include <QStandardPaths>

#include <iostream>
#include <QDir>

#include <nlohmann/json.hpp>

// for convenience
using json = nlohmann::json;
using namespace kainjow::mustache;

int
main(int argc, char* argv[])
{
  int rv;

  // Go to users home directory
  QDir::setCurrent(QDir::homePath());

  btest app(argc, argv);
  QApplication::setApplicationName("vscpsim");
  QApplication::setApplicationVersion("0.1");

  // Load configuration file
  app.loadProgramSettings();

  QCommandLineParser parser;
  parser.setApplicationDescription("VSCP Simulator");
  parser.addHelpOption();
  parser.addVersionOption();

  // --------------------------------------------------------------------------

  // interface (-i, --interface)
  QCommandLineOption interfaceOption(
    QStringList() << "i"
                  << "interface",
    QApplication::translate("main", "Client interface to use"),
    "interface (socketcan, canal, tcpip, udp, multicast, mqtt, ws1, ws2)",
    "socketcan");
  parser.addOption(interfaceOption);

  // config (-b, --bootmode)
  QCommandLineOption bootOption(
    QStringList() << "b"
                  << "bootmode",
    QApplication::translate("main", "Start bootloader (non zero, typical 0xff) or application firmware (0)"),
    "mode",
    "0xff");
  parser.addOption(bootOption);

  // config (-B, --block)
  QCommandLineOption blockOption(
    QStringList() << "B"
                  << "block"
                  << "blocksize",
    QApplication::translate("main", "Block info on the form 'size:count' where size is he size "
                                    "of a block in bytes and count are the number of blocks of that size."),
    "blck:cnt",
    "");
  parser.addOption(blockOption);

  // config (-c, --config)
  QCommandLineOption configOption(
    QStringList() << "c"
                  << "config",
    QApplication::translate("main", "Configuration string for interface (see docs for interface)"),
    "cfg1;cfg2;cfg2;...",
    "vcan0");
  parser.addOption(configOption);

  // GUID (-g, --guid)
  QCommandLineOption guidOption(
    QStringList() << "g"
                  << "guid",
    QApplication::translate("main", "GUID to use for this client"),
    "guid",
    "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00");
  parser.addOption(guidOption);

  // host (-h, --host)
  QCommandLineOption hostOption(
    QStringList() << "s"
                  << "srv"
                  << "host"
                  << "server",
    QApplication::translate("main", "Server/Host to connect to"),
    "host",
    "localhost");
  parser.addOption(hostOption);

  // port (-p, --port)
  QCommandLineOption portOption(
    QStringList() << "p"
                  << "port"
                  << "remote-port",
    QApplication::translate("main", "Port to connect to"),
    "port",
    "9598");
  parser.addOption(portOption);

  // timeout (-t, --timeout)
  QCommandLineOption timeoutOption(QStringList() << "t"
                                                 << "timeout",
                                   QApplication::translate("main", "Timeout in milliseconds for communiction"),
                                   "ms",
                                   "1000");
  parser.addOption(timeoutOption);

  // user (-u, --user)
  QCommandLineOption userOption(QStringList() << "u"
                                              << "user",
                                QApplication::translate("main", "Username"),
                                "user",
                                "vscp");
  parser.addOption(userOption);

  // password (-p, --password)
  QCommandLineOption passwordOption(QStringList() << "P"
                                                  << "password",
                                    QApplication::translate("main", "Password"),
                                    "password",
                                    "secret");
  parser.addOption(passwordOption);

  // VSCP level (-l, --level)
  QCommandLineOption levelOption(QStringList() << "l"
                                               << "vscplevel",
                                 QApplication::translate("main", "VSCP protocol level"),
                                 "(0|1)",
                                 "0");
  parser.addOption(levelOption);

  // Configuration file
  QCommandLineOption fileOption(QStringList() << "f"
                                              << "cfgfile",
                                QApplication::translate("main", "Configuration file path"),
                                "path",
                                "");
  parser.addOption(fileOption);

  // --------------------------------------------------------------------------

  /*!
   * Load flash from file.
   * Load registers from MDF file.
   * Simulation schema.
   */

  // Process the actual command line arguments given by the user
  parser.process(app);

  const QStringList args = parser.positionalArguments();
  // source is args.at(0), destination is args.at(1)

  QString iface   = parser.value(interfaceOption);
  app.m_interface = iface;
  app.m_interface = app.m_interface.trimmed();
  app.m_interface = app.m_interface.toLower();
  spdlog::debug("Config: interface: {}", app.m_interface.toStdString());

  QString cfg  = parser.value(configOption);
  app.m_config = cfg;
  vscp_split(app.m_configVector, cfg.toStdString(), ";");
  int idx = 0;
  for (const auto& cfgopt : app.m_configVector) {
    spdlog::debug("Config: config string item {0}: {1} [2]", idx, cfgopt, app.m_config.toStdString());
    idx++;
  }

  QString bootstr = parser.value(bootOption);
  app.m_bootflag  = vscp_readStringValue(bootstr.toStdString()); // app.BOOTLOADER;
  spdlog::debug("Config: bootflag: {}", app.m_bootflag);

  QString hoststr = parser.value(hostOption);
  app.m_host      = hoststr;
  spdlog::debug("Config: Host set to: {}", app.m_host.toStdString());

  QString portstr = parser.value(portOption);
  app.m_port      = vscp_readStringValue(portstr.toStdString());
  spdlog::debug("Config: port: {}", app.m_port);

  QString userstr = parser.value(userOption);
  app.m_user      = userstr;
  spdlog::debug("Config: user: {}", app.m_user.toStdString());

  QString passwordstr = parser.value(passwordOption);
  app.m_password      = passwordstr;
  spdlog::debug("Config: password: {}", app.m_password.toStdString());

  std::string blockstr = parser.value(blockOption).toStdString();
  blockstr             = vscp_trim_copy(blockstr);
  if (blockstr.size()) {
    std::deque<std::string> blockVector;
    vscp_split(blockVector, blockstr, ":");
    if (blockVector.size() > 1) {
      app.m_bootloader_cfg.blockSize  = vscp_readStringValue(blockVector[0]);
      app.m_bootloader_cfg.blockCount = vscp_readStringValue(blockVector[1]);
      spdlog::debug("Config: block: size={0} count={1}",
                    app.m_bootloader_cfg.blockSize,
                    app.m_bootloader_cfg.blockCount);
    }
  }

  QString levelstr = parser.value(levelOption);
  if (vscp_readStringValue(levelstr.toStdString()) <= 2) {
    app.m_bootloader_cfg.vscpLevel = vscp_readStringValue(levelstr.toStdString());
    spdlog::debug("Config: level: {}", app.m_bootloader_cfg.vscpLevel);
  }

  app.m_configpath = parser.value(fileOption).trimmed();
  if (app.m_configpath.size() && !vscp_fileExists(app.m_configpath.toStdString())) {
    spdlog::error("Configuration file does not exist ('{}')", app.m_configpath.toStdString());
    exit(1);
  }

  spdlog::debug("Log to file: {}", app.m_fileLogPath);

  // Load configuration file if one is specified
  if (app.m_configpath.size()) {
    rv = app.loadFirmwareConfig(app.m_configpath);
  }

  // Initialize simulation memory
  // app.initSimulationData();

  // Init the interface/hardware
  // if (VSCP_ERROR_SUCCESS != (rv = app.vscpboot_init_hardware())) {
  //   spdlog::error("Main: Failed to init hardware, rv={}", rv);
  //   return -2;
  // }

  spdlog::debug("Hardware initialized OK");

  // --------------------------------------------------------------------------

  do {

    // Start bootloader (will in turn start app if boot flag is zero)
    if (VSCP_ERROR_SUCCESS != (rv = app.startWorkerThread())) {
      spdlog::error("Main: startWorkerThread {}", rv);
      return 0;
    }

    MainWindow mainWindow;
    mainWindow.show();
    rv = app.exec();

    // Start bootloader (will in turn start app if boot flag is zero)
    if (VSCP_ERROR_SUCCESS != (rv = app.stopWorkerThread())) {
      spdlog::error("Main: stopWorkerThread {}", rv);
      return rv;
    }
  } while (!app.m_bEndNormal); 

  return rv;
}
