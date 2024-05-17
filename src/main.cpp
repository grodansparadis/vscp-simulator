#include "btest.h"
#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

#include <iostream>

int
main(int argc, char* argv[])
{
  int rv;
  btest app(argc, argv);
  QApplication::setApplicationName("btest");
  QApplication::setApplicationVersion("0.1");

  QCommandLineParser parser;
  parser.setApplicationDescription("VSCP Boot Test helper");
  parser.addHelpOption();
  parser.addVersionOption();

  // interface (-i, --interface)
  QCommandLineOption interfaceOption(
    QStringList() << "i"
                  << "interface",
    QApplication::translate("main", "Client interface to use"),
    "interface",
    "socketcan");
  parser.addOption(interfaceOption);

  // config (-b, --bootmode)
  QCommandLineOption bootOption(
    QStringList() << "b"
                  << "bootmode",
    QApplication::translate("main", "Start bootloader (non zero) or application firmware (0)"),
    "mode",
    "255");
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
    QApplication::translate("main", "Configuration string for interface"),
    "cfg1;cfg2;cfg2;...",
    "vcan0");
  parser.addOption(configOption);

  // GUID (-g, --guid)
  QCommandLineOption guidOption(
    QStringList() << "g"
                  << "guid",
    QApplication::translate("main", "GUID to use for this client"),
    "guid",
    "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:01");
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
    "1883");
  parser.addOption(portOption);

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
                                 QApplication::translate("main", "Level"),
                                 "(0|1)",
                                 "1");
  parser.addOption(levelOption);

  /*!
   * Load flash from file.
   * Load registers from MDF file.
   * Simulation schema.
   */

  // Process the actual command line arguments given by the user
  parser.process(app);

  const QStringList args = parser.positionalArguments();
  // source is args.at(0), destination is args.at(1)

  QString iface = parser.value(interfaceOption);
  // std::cout << "Interface = " << iface.toStdString() << std::endl;
  app.m_interface = iface;
  app.m_interface = app.m_interface.trimmed();
  app.m_interface = app.m_interface.toLower();

  QString cfg = parser.value(configOption);
  // std::cout << "Configuration = " << cfg.toStdString() << std::endl;
  app.m_config = cfg;
  vscp_split(app.m_configVector, cfg.toStdString(), ";");
  if (app.m_configVector.size()) {
    std::cout << "Configuration = " << app.m_configVector[0] << std::endl;
  }

  QString bootstr = parser.value(bootOption);
  // std::cout << "Host = " << host.toStdString() << std::endl;
  app.m_bootflag = vscp_readStringValue(bootstr.toStdString()); // app.BOOTLOADER;

  QString hoststr = parser.value(hostOption);
  // std::cout << "Host = " << host.toStdString() << std::endl;
  app.m_host = hoststr;

  QString portstr = parser.value(portOption);
  // std::cout << "Port = " << port << std::endl;
  app.m_port = vscp_readStringValue(portstr.toStdString());

  QString userstr = parser.value(userOption);
  // std::cout << "User = " << user.toStdString() << std::endl;
  app.m_user = userstr;

  QString passwordstr = parser.value(passwordOption);
  // std::cout << "Password = " << password.toStdString() << std::endl;
  app.m_password = passwordstr;

  std::string blockstr = parser.value(blockOption).toStdString();
  blockstr             = vscp_trim_copy(blockstr);
  if (blockstr.size()) {
    std::deque<std::string> blockVector;
    vscp_split(blockVector, blockstr, ":");
    if (blockVector.size() > 1) {
      app.m_bootloader_cfg.blockSize  = vscp_readStringValue(blockVector[0]);
      app.m_bootloader_cfg.blockCount = vscp_readStringValue(blockVector[1]);
    }
  }

  QString levelstr               = parser.value(passwordOption);
  app.m_bootloader_cfg.vscpLevel = vscp_readStringValue(levelstr.toStdString());

  // Start bootloader (will in turn start app if boot flag is zero)
  if (VSCP_ERROR_SUCCESS != (rv = app.startWorkerThread())) {
    printf("Main: Send error: rv=%d\n", rv);
    return 0;
  }

  MainWindow mainWindow;
  mainWindow.show();
  return app.exec();
}
