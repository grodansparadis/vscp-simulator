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

  if (VSCP_ERROR_SUCCESS != (rv = vscpboot_init_hardware())) {
    spdlog::error("Failed to init. hardware rv={}", rv);
    return;
  }

  vscpEventEx ex;
  memset(&ex, 0, sizeof(vscpEventEx));

  // Confirm bootmode - Send bootmode ACK
  ex.head       = VSCP_PRIORITY_HIGH;
  ex.sizeData   = 8;
  ex.vscp_class = VSCP_CLASS1_PROTOCOL;
  ex.vscp_type  = VSCP_TYPE_PROTOCOL_ACK_BOOT_LOADER;
  ex.data[0]    = 1;
  ex.data[1]    = 2;
  ex.data[2]    = 3;
  ex.data[3]    = 4;
  ex.data[4]    = 5;
  ex.data[5]    = 6;
  ex.data[6]    = 7;
  ex.data[7]    = 8;
  vscpboot_sendEvent(&ex);
}

MainWindow::~MainWindow()
{
  int rv;

  if (VSCP_ERROR_SUCCESS != (rv = vscpboot_release_hardware())) {
    spdlog::error("Failed to release hardware rv={}", rv);
    return;
  }
}
