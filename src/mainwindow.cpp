#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGlobal>

#include "btest.h"

#include <vscp.h>
#include <vscphelper.h>
#include <vscp-bootloader.h>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), ui(new Ui::MainWindow)
{
  int rv;

  ui->setupUi(this);

  btest* pbtest = (btest*)QApplication::instance();
  ui->lblQtVersion->setText(qVersion());
  ui->lblInterface->setText(pbtest->m_interface);
  ui->lblConfiguration->setText(pbtest->m_config);
  ui->lblHost->setText(pbtest->m_host);
  ui->lblPort->setText(QString("%1").arg(pbtest->m_port));
  ui->lblUser->setText(pbtest->m_user);
  ui->lblPassword->setText(pbtest->m_password);
  ui->lblGuid->setText(pbtest->m_guid.toString().c_str());
}

MainWindow::~MainWindow()
{
  
}
