#include "btest.h"
#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineParser>

#include <iostream>

int
main(int argc, char* argv[])
{
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

  // config (-c, --config)
  QCommandLineOption configOption(
    QStringList() << "c"
                  << "config",
    QApplication::translate("main", "Configuration string for interface"),
    "config",
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
                                              << "user"
                                              << "user",
                                QApplication::translate("main", "Username"),
                                "user",
                                "vscp");
  parser.addOption(userOption);

  // password (-p, --password)
  QCommandLineOption passwordOption(QStringList() << "P"
                                                  << "password"
                                                  << "password",
                                    QApplication::translate("main", "Password"),
                                    "password",
                                    "secret");
  parser.addOption(passwordOption);

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

  QString host = parser.value(hostOption);
  // std::cout << "Host = " << host.toStdString() << std::endl;
  app.m_host = host;

  uint16_t port = parser.value(portOption).toInt();
  // std::cout << "Port = " << port << std::endl;
  app.m_port = port;

  QString user = parser.value(userOption);
  // std::cout << "User = " << user.toStdString() << std::endl;
  app.m_user = user;

  QString password = parser.value(passwordOption);
  // std::cout << "Password = " << password.toStdString() << std::endl;
  app.m_password = password;

  MainWindow mainWindow;
  mainWindow.show();
  return app.exec();
}
