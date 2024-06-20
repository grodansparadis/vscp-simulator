#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGlobal>

#include "btest.h"

#include <vscp-bootloader.h>
#include <vscp.h>
#include <vscphelper.h>

#include <spdlog/async.h>
#include <spdlog/sinks/qt_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  int rv;

  ui->setupUi(this);

  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  // Log
  connect(ui->btnClearLog, SIGNAL(clicked()), this, SLOT(clearLog()));
  connect(ui->btnSave, SIGNAL(clicked()), this, SLOT(saveLog()));
  connect(ui->comboLogLevel, SIGNAL(currentIndexChanged(int)), this, SLOT(setLogLevel(int)));

  // Tools
  connect(ui->actionRestDevice, SIGNAL(triggered()), this, SLOT(resetDevice()));
  connect(ui->actionSetBootMode, SIGNAL(triggered()), this, SLOT(setBootMode()));
  connect(ui->actionSetFirmwareMode, SIGNAL(triggered()), this, SLOT(setFirmwareMode()));

  // Simulation controls
  connect(papp, &btest::checkValueChanged, this, &MainWindow::setCheckboxValue);
  connect(papp, &btest::sliderValueChanged, this, &MainWindow::setSliderValue);
  connect(papp, &btest::radioValueChanged, this, &MainWindow::setRadioButtonValue);
  connect(papp, &btest::backgroundColorChanged, this, &MainWindow::setBackgroundColor);

  //////////////////////////////////////////////////////////////////////////////
  //                                spdlog
  //////////////////////////////////////////////////////////////////////////////

  // patterns - https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  // https://github.com/gabime/spdlog/wiki/2.-Creating-loggers#creating-loggers-with-multiple-sinks

  try {

    // create console_sink
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(papp->m_consoleLogLevel);
    console_sink->set_pattern(papp->m_consoleLogPattern);

    // create rotating file sink
    auto file_sink =
      std::make_shared<spdlog::sinks::rotating_file_sink_mt>(papp->m_fileLogPath,
                                                             papp->m_maxFileLogSize,
                                                             papp->m_maxFileLogFiles,
                                                             true);
    file_sink->set_level(papp->m_fileLogLevel);
    file_sink->set_pattern(papp->m_fileLogPattern);

    // https://stackoverflow.com/questions/66531834/get-log-from-spdlog-in-qtextedit-object-from-qt
    // auto qtlogger = spdlog::qt_logger_mt("btest_logger", ui->txtEditLog);
    m_qt_sink = std::make_shared<spdlog::sinks::qt_color_sink_mt>(ui->txtEditLog, 2048, true);

    // sink's bucket
    spdlog::sinks_init_list sinks{ console_sink, file_sink, m_qt_sink };

    // create async logger, and use global threadpool
    spdlog::init_thread_pool(1024 * 8, 1);
    auto logger = std::make_shared<spdlog::async_logger>("logger", sinks, spdlog::thread_pool());

    // set default logger
    spdlog::set_default_logger(logger);
    spdlog::set_level(papp->m_fileLogLevel);
  }
  catch (...) {
    fprintf(stderr, "Unable to init logsystem. Logs Exiting.");
    spdlog::drop_all();
    spdlog::shutdown();
    exit(EXIT_FAILURE);
  }

  // --------------------------------------------------------------------------

  // init application
  init();
}

MainWindow::~MainWindow()
{
}

///////////////////////////////////////////////////////////////////////////////
// init
//

void
MainWindow::init(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  ui->lblQtVersion->setText(qVersion());
  ui->lblLevel->setText((VSCP_LEVEL1 == papp->m_firmware_cfg.m_level) ? "Level I" : "Level II");
  ui->lblInterface->setText(papp->m_interface);
  ui->lblConfiguration->setText(papp->m_config);
  ui->lblHost->setText(papp->m_host);
  ui->lblPort->setText(QString("%1").arg(papp->m_port));
  ui->lblUser->setText(papp->m_user);
  ui->lblPassword->setText(papp->m_password);
  ui->lblGuid->setText(papp->m_guid.toString().c_str());

  if (0xff == papp->m_bootflag) {
    setBootLoaderModeUi();
    ui->lblMode->setText("Simulated bootloader");
  }
  else {
    setFirmwareModeUi();
    ui->lblMode->setText("Simulated firmware");
  }
}

///////////////////////////////////////////////////////////////////////////////
// clearLog
//

void
MainWindow::clearLog(void)
{
  spdlog::info("Clear log");
  ui->txtEditLog->clear();
}

///////////////////////////////////////////////////////////////////////////////
// saveLog
//

void
MainWindow::saveLog(void)
{
  spdlog::info("save log");
}

///////////////////////////////////////////////////////////////////////////////
// setLogLevel
//

void
MainWindow::setLogLevel(int level)
{
  m_qt_sink->set_level(static_cast<spdlog::level::level_enum>(level));
}

///////////////////////////////////////////////////////////////////////////////
// setFirmwareModeUi
//

void
MainWindow::setFirmwareModeUi(void)
{
  // Set background color for firmware mode
  // this->setStyleSheet("background-color: rgb(255, 255, 255);");
  this->setStyleSheet("selection-background-color: rgb(0, 0, 0);");
  ui->tabWidget->setTabVisible(tab_index_register, true);
  ui->tabWidget->setTabVisible(tab_index_simulation, true);
}

///////////////////////////////////////////////////////////////////////////////
// setBootLoaderModeUi
//

void
MainWindow::setBootLoaderModeUi(void)
{
  // Set background color for firmware mode
  this->setStyleSheet("background-color: rgb(222, 255, 255);selection-background-color: rgb(0, 0, 0);");
  ui->tabWidget->setTabVisible(tab_index_register, false);
  ui->tabWidget->setTabVisible(tab_index_simulation, false);
}

///////////////////////////////////////////////////////////////////////////////
// resetDevice
//

void
MainWindow::resetDevice(void)
{
  int rv;
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::trace("resetDevice");

  // Init simulation window
  init();

  if (VSCP_ERROR_SUCCESS != (rv = papp->stopWorkerThread())) {
    spdlog::error("Stopping simulation failed. rv=%s", rv);
  }

  if (VSCP_ERROR_SUCCESS != (rv = papp->startWorkerThread())) {
    spdlog::error("Starting simulation failed. rv=%s", rv);
  }
}

///////////////////////////////////////////////////////////////////////////////
// setBootMode
//

void
MainWindow::setBootMode(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  papp->m_bootflag = 0xff; // Set bootmode
  resetDevice();           // Reset the device
}

///////////////////////////////////////////////////////////////////////////////
// setBootMode
//

void
MainWindow::setFirmwareMode(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  papp->m_bootflag = 0x00; // Set firmware mode
  resetDevice();           // Reset the device
}

///////////////////////////////////////////////////////////////////////////////
// setCheckboxValue
//

void MainWindow::setCheckboxValue(int idx, bool value)
{
  switch (idx) {
    case 0:
      ui->chk0->setChecked(value);
      break;
    case 1:
      ui->chk1->setChecked(value);
      break;
    case 2:
      ui->chk2->setChecked(value);
      break;
    case 3:
      ui->chk3->setChecked(value);
      break;
    case 4:
      ui->chk4->setChecked(value);
      break;
    case 5:
      ui->chk5->setChecked(value);
      break;
    case 6:
      ui->chk6->setChecked(value);
      break;
    case 7:
      ui->chk7->setChecked(value);
      break;
    case 8:
      ui->chk8->setChecked(value);
      break;
    case 9:
      ui->chk9->setChecked(value);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// setSliderValue
//

void
MainWindow::setSliderValue(int idx, uint8_t value)
{
  switch (idx) {
    case 0:
      ui->slider0->setValue(value);
      break;
    case 1:
      ui->slider1->setValue(value);
      break;
    case 2:
      ui->slider2->setValue(value);
      break;
    case 3:
      ui->slider3->setValue(value);
      break;
    case 4:
      ui->slider4->setValue(value);
      break;
    case 5:
      ui->slider5->setValue(value);
      break;
    case 6:
      ui->slider6->setValue(value);
      break;
    case 7:
      ui->slider7->setValue(value);
      break;
    case 8:
      ui->slider8->setValue(value);
      break;
    case 9:
      ui->slider9->setValue(value);
      break;
  }
}


///////////////////////////////////////////////////////////////////////////////
// setRadioButtonValue
//

void MainWindow::setRadioButtonValue(int idx, bool value)
{
  switch (idx) {
    case 0:
      ui->radio0->setChecked(value);
      break;
    case 1:
      ui->radio1->setChecked(value);
      break;
    case 2:
      ui->radio2->setChecked(value);
      break;
    case 3:
      ui->radio3->setChecked(value);
      break;
    case 4:
      ui->radio4->setChecked(value);
      break;
    case 5:
      ui->radio5->setChecked(value);
      break;
    case 6:
      ui->radio6->setChecked(value);
      break;
    case 7:
      ui->radio7->setChecked(value);
      break;
    case 8:
      ui->radio8->setChecked(value);
      break;
    case 9:
      ui->radio9->setChecked(value);
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////
// setBackgroundColor
//

void MainWindow::setBackgroundColor(uint32_t color)
{
  QString strcolor = 
    QString("background-color: rgb(%0,%1,%2);").arg((color >> 16) & 0xff).arg((color >> 8) & 0xff).arg(color & 0xff);
  //ui->tab_sim->setStyleSheet("background-color: rgb(255, 0, 0);");
  ui->tab_sim->setStyleSheet(strcolor);
}