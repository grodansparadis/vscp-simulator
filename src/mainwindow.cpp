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
  //int rv;

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

  connect(papp, &btest::initRegisters, this, &MainWindow::initRegisters);
  connect(papp, &btest::updateRegister, this, &MainWindow::updateRegister);

  connect(ui->btn0, SIGNAL(pressed()), this, SLOT(btnPressed_s0()));
  connect(ui->btn1, SIGNAL(pressed()), this, SLOT(btnPressed_s1()));
  connect(ui->btn2, SIGNAL(pressed()), this, SLOT(btnPressed_s2()));
  connect(ui->btn3, SIGNAL(pressed()), this, SLOT(btnPressed_s3()));
  connect(ui->btn4, SIGNAL(pressed()), this, SLOT(btnPressed_s4()));
  connect(ui->btn5, SIGNAL(pressed()), this, SLOT(btnPressed_s5()));
  connect(ui->btn6, SIGNAL(pressed()), this, SLOT(btnPressed_s6()));
  connect(ui->btn7, SIGNAL(pressed()), this, SLOT(btnPressed_s7()));
  connect(ui->btn8, SIGNAL(pressed()), this, SLOT(btnPressed_s8()));
  connect(ui->btn9, SIGNAL(pressed()), this, SLOT(btnPressed_s9()));

  connect(ui->chk0, SIGNAL(clicked()), this, SLOT(chkClicked_c0()));
  connect(ui->chk1, SIGNAL(clicked()), this, SLOT(chkClicked_c1()));
  connect(ui->chk2, SIGNAL(clicked()), this, SLOT(chkClicked_c2()));
  connect(ui->chk3, SIGNAL(clicked()), this, SLOT(chkClicked_c3()));
  connect(ui->chk4, SIGNAL(clicked()), this, SLOT(chkClicked_c4()));
  connect(ui->chk5, SIGNAL(clicked()), this, SLOT(chkClicked_c5()));
  connect(ui->chk6, SIGNAL(clicked()), this, SLOT(chkClicked_c6()));
  connect(ui->chk7, SIGNAL(clicked()), this, SLOT(chkClicked_c7()));
  connect(ui->chk8, SIGNAL(clicked()), this, SLOT(chkClicked_c8()));
  connect(ui->chk9, SIGNAL(clicked()), this, SLOT(chkClicked_c9()));

  connect(ui->radio0, &QRadioButton::toggled, this, &MainWindow::radioClicked_r0);
  connect(ui->radio1, &QRadioButton::toggled, this, &MainWindow::radioClicked_r1);
  connect(ui->radio2, &QRadioButton::toggled, this, &MainWindow::radioClicked_r2);
  connect(ui->radio3, &QRadioButton::toggled, this, &MainWindow::radioClicked_r3);
  connect(ui->radio4, &QRadioButton::toggled, this, &MainWindow::radioClicked_r4);
  connect(ui->radio5, &QRadioButton::toggled, this, &MainWindow::radioClicked_r5);
  connect(ui->radio6, &QRadioButton::toggled, this, &MainWindow::radioClicked_r6);
  connect(ui->radio7, &QRadioButton::toggled, this, &MainWindow::radioClicked_r7);
  connect(ui->radio8, &QRadioButton::toggled, this, &MainWindow::radioClicked_r8);
  connect(ui->radio9, &QRadioButton::toggled, this, &MainWindow::radioClicked_r9);

  connect(ui->slider0, &QSlider::valueChanged, this, &MainWindow::sliderChanged_s0);
  connect(ui->slider1, &QSlider::valueChanged, this, &MainWindow::sliderChanged_s1);
  connect(ui->slider2, &QSlider::valueChanged, this, &MainWindow::sliderChanged_s2);
  connect(ui->slider3, &QSlider::valueChanged, this, &MainWindow::sliderChanged_s3);
  connect(ui->slider4, &QSlider::valueChanged, this, &MainWindow::sliderChanged_s4);
  connect(ui->slider5, &QSlider::valueChanged, this, &MainWindow::sliderChanged_s5);
  connect(ui->slider6, &QSlider::valueChanged, this, &MainWindow::sliderChanged_s6);
  connect(ui->slider7, &QSlider::valueChanged, this, &MainWindow::sliderChanged_s7);
  connect(ui->slider8, &QSlider::valueChanged, this, &MainWindow::sliderChanged_s8);
  connect(ui->slider9, &QSlider::valueChanged, this, &MainWindow::sliderChanged_s9);

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

  // Initilize simulation data storage
  papp->initSimulationData();

  ui->lblPrgVersion->setText(QString("%1.%2.%3")
                               .arg(APP_VERSION_MAJOR)
                               .arg(APP_VERSION_MINOR)
                               .arg(APP_VERSION_RELEASE));
  ui->lblQtVersion->setText(qVersion());
  ui->lblLevel->setText((VSCP_LEVEL1 == papp->m_firmware_cfg.m_level) ? "Level I" : "Level II");
  ui->lblInterface->setText(papp->m_interface);
  ui->lblConfiguration->setText(papp->m_config);
  ui->lblHost->setText(papp->m_host);
  ui->lblPort->setText(QString("%1").arg(papp->m_port));
  ui->lblUser->setText(papp->m_user);
  ui->lblPassword->setText(papp->m_password);
  cguid guid(papp->m_firmware_cfg.m_guid);
  ui->lblGuid->setText(/*papp->m_guid*/ guid.toString().c_str());

  if (0xff == papp->m_bootflag) {
    setBootLoaderModeUi();
    ui->lblMode->setText("Simulated bootloader");
  }
  else {
    setFirmwareModeUi();
    ui->lblMode->setText("Simulated firmware");
  }

  // Registers
  initRegisters();

  // RGB Colors
  updateRegister(82, 0, 0xff);
  updateRegister(83, 0, 0xff);
  updateRegister(84, 0, 0xff);

  // Standard registers
  papp->standardRegHasChanged(VSCP_STD_REGISTER_ALARM_STATUS);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_MAJOR_VERSION);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_MINOR_VERSION);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_ERROR_COUNTER);
  for (int i = 0; i <= 4; i++) {
    papp->standardRegHasChanged(VSCP_STD_REGISTER_USER_ID + i);
  }
  for (int i = 0; i <= 4; i++) {
    papp->standardRegHasChanged(VSCP_STD_REGISTER_USER_MANDEV_ID + i);
  }
  for (int i = 0; i <= 4; i++) {
    papp->standardRegHasChanged(VSCP_STD_REGISTER_USER_MANSUBDEV_ID + i);
  }
  papp->standardRegHasChanged(VSCP_STD_REGISTER_PAGE_SELECT_MSB);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_PAGE_SELECT_LSB);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_FIRMWARE_MAJOR);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_FIRMWARE_MINOR);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_FIRMWARE_SUBMINOR);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_BOOT_LOADER);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_BUFFER_SIZE);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_PAGES_COUNT);
  for (int i = 0; i <= 4; i++) {
    papp->standardRegHasChanged(VSCP_STD_REGISTER_FAMILY_CODE + i);
  }
  for (int i = 0; i <= 4; i++) {
    papp->standardRegHasChanged(VSCP_STD_REGISTER_DEVICE_TYPE + i);
  }
  papp->standardRegHasChanged(VSCP_STD_REGISTER_NODE_RESET);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_FIRMWARE_CODE_MSB);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_FIRMWARE_CODE_LSB);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_NICKNAME_ID_LSB);
  papp->standardRegHasChanged(VSCP_STD_REGISTER_NICKNAME_ID_MSB);

  for (int i = 0; i < 16; i++) {
    papp->standardRegHasChanged(VSCP_STD_REGISTER_GUID + i);
  }

  for (int i = 0; i < 32; i++) {
    papp->standardRegHasChanged(VSCP_STD_REGISTER_DEVICE_URL + i);
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
  this->setStyleSheet("background-color: rgb(255, 255, 255);selection-background-color: rgb(0, 0, 0);");
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
// addRegRow
//
// void
// MainWindow::addRegRow(uint32_t row, uint16_t page, uint8_t value)
// {
//   QString str("register %1:\t %2\t%3\t%4");
//   uint32_t combined = ((uint32_t)page << 16) + row; // If page > 0 row is < 128

//   // QListWidgetItem *pitem = new QListWidgetItem(str.arg(row).arg(value, 4, 10, QChar(' ')).arg(value, 4, 16, QChar('0')).arg(value, 8, 2, QChar('0')));
//   //  QListWidgetItem *pitem = new QListWidgetItem("test");
//   //  ui->registerList->addItem(pitem);
//   spdlog::error("Register {0}:{2}", page, row);
// }

///////////////////////////////////////////////////////////////////////////////
// updateRegRow
//

void
MainWindow::updateRegRow(uint32_t row, uint16_t page, uint8_t value)
{
  uint32_t combined = ((uint32_t)page << 16) + row; // If page > 0 row is < 128
  spdlog::debug("updateRegRow: {0}", combined);
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

void
MainWindow::setCheckboxValue(int idx, bool value)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  switch (idx) {

    case 0:
      ui->chk0->setChecked(value);
      papp->checkboxClick(0, value);
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

void
MainWindow::setRadioButtonValue(int idx, bool value)
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
// initRegisters
//
//

void
MainWindow::initRegisters(void)
{
  uint16_t page = 0;
  uint32_t offset;
  QString str;

  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  std::map<uint32_t, QListWidgetItem*>::iterator it;
  for (it = papp->m_regmap.begin(); it != papp->m_regmap.end(); ++it) {

    if (VSCP_LEVEL1 == papp->m_firmware_cfg.m_level) {
      // Level I
      page   = it->first >> 16;
      offset = it->first & 0xffff;
    }
    else {
      // Level II
      offset = it->first;
    }

    QListWidgetItem* pitem = new QListWidgetItem();
    if (nullptr == pitem) {
      spdlog::error("initRegisters: Memory error");
      return;
    }

    if (VSCP_LEVEL1 == papp->m_firmware_cfg.m_level) {
      if ((0 == page) && (offset >= 128)) {
        str = "Standard reg %1:%2:\t %3\t0x%4\t0b%5";
        QBrush brush(QColor(0, 255, 0, 12));
        pitem->setBackground(brush);
      }
      else {
        str = "User reg %1:%2:\t %3\t0x%4\t0b%5";
        QBrush brush(QColor(0, 0, 255, 12));
        pitem->setBackground(brush);
      }
    }
    else {
      if (offset < 0xffff0000) {
        str = "User reg %1:%2:\t %3\t0x%4\t0b%5";
        QBrush brush(QColor(0, 0, 255, 12));
        pitem->setBackground(brush);
      }
      else {
        str = "Standard reg %1:%2:\t %3\t0x%4\t0b%5";
        QBrush brush(QColor(0, 255, 0, 12));
        pitem->setBackground(brush);
      }
    }

    pitem->setText(str.arg(page)
                     .arg(offset)
                     .arg(0, 4, 10, QChar(' '))
                     .arg(0, 2, 16, QChar('0'))
                     .arg(0, 8, 2, QChar('0')));

    ui->registerList->addItem(pitem);

    // Map list item
    papp->m_regmap[offset] = pitem;
  }
}

///////////////////////////////////////////////////////////////////////////////
// updateRegister
//

void
MainWindow::updateRegister(uint32_t offset, uint16_t page, uint8_t value)
{
  QString str;

  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  QListWidgetItem* pitem = papp->m_regmap[((uint32_t)page << 16) + offset];
  if (nullptr == pitem) {
    spdlog::error("Register item is invalid (NULL) offset={1} page){2}", offset, page);
    return;
  }

  if (VSCP_LEVEL1 == papp->m_firmware_cfg.m_level) {
    if ((0 == page) && (offset >= 128)) {
      str = "Standard reg %1:%2:\t %3\t0x%4\t0b%5";
      QBrush brush(QColor(0, 255, 0, 48));
      pitem->setBackground(brush);
    }
    else {
      str = "User reg %1:%2:\t %3\t0x%4\t0b%5";
      QBrush brush(QColor(0, 0, 255, 48));
      pitem->setBackground(brush);
    }
  }
  else {
    if (offset < 0xffff0000) {
      str = "User reg %1:%2:\t %3\t0x%4\t0b%5";
      QBrush brush(QColor(0, 0, 255, 48));
      pitem->setBackground(brush);
    }
    else {
      str = "Standard reg %1:%2:\t %3\t0x%4\t0b%5";
      QBrush brush(QColor(0, 255, 0, 48));
      pitem->setBackground(brush);
    }
  }

  str = str.arg(page)
          .arg(offset)
          .arg(value, 4, 10, QChar(' '))
          .arg(value, 2, 16, QChar('0'))
          .arg(value, 8, 2, QChar('0'));
  pitem->setText(str);
}

///////////////////////////////////////////////////////////////////////////////
// setBackgroundColor
//

void
MainWindow::setBackgroundColor(uint32_t color)
{
  QString strcolor =
    QString("background-color: rgb(%0,%1,%2);").arg((color >> 16) & 0xff).arg((color >> 8) & 0xff).arg(color & 0xff);
  ui->tab_sim->setStyleSheet(strcolor);
  spdlog::info("Set color {0} {1:08X}", strcolor.toStdString(), (int)color);
}

//=============================================================================
//                                 BUTTONS
//=============================================================================

///////////////////////////////////////////////////////////////////////////////
// btnPressed_s0
//

void
MainWindow::btnPressed_s0(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Button 0 pressed");
  papp->buttonPress(0);
}

///////////////////////////////////////////////////////////////////////////////
// btnPressed_s1
//

void
MainWindow::btnPressed_s1(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Button 1 pressed");
  papp->buttonPress(1);
}

///////////////////////////////////////////////////////////////////////////////
// btnPressed_s2
//

void
MainWindow::btnPressed_s2(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Button 2 pressed");
  papp->buttonPress(2);
}

///////////////////////////////////////////////////////////////////////////////
// btnPressed_s3
//

void
MainWindow::btnPressed_s3(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Button 3 pressed");
  papp->buttonPress(3);
}

///////////////////////////////////////////////////////////////////////////////
// btnPressed_s4
//

void
MainWindow::btnPressed_s4(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Button 4 pressed");
  papp->buttonPress(4);
}

///////////////////////////////////////////////////////////////////////////////
// btnPressed_s5
//

void
MainWindow::btnPressed_s5(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Button 5 pressed");
  papp->buttonPress(5);
}

///////////////////////////////////////////////////////////////////////////////
// btnPressed_s6
//

void
MainWindow::btnPressed_s6(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Button 6 pressed");
  papp->buttonPress(6);
}

///////////////////////////////////////////////////////////////////////////////
// btnPressed_s7
//

void
MainWindow::btnPressed_s7(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Button 7 pressed");
  papp->buttonPress(7);
}

///////////////////////////////////////////////////////////////////////////////
// btnPressed_s8
//

void
MainWindow::btnPressed_s8(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Button 8 pressed");
  papp->buttonPress(8);
}

///////////////////////////////////////////////////////////////////////////////
// btnPressed_s9
//

void
MainWindow::btnPressed_s9(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Button 9 pressed");
  papp->buttonPress(9);
}

//=============================================================================
//                                 CHECKBOX
//=============================================================================

///////////////////////////////////////////////////////////////////////////////
// chkClicked_c0
//

void
MainWindow::chkClicked_c0(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox c0 clicked");
  papp->checkboxClick(0, ui->chk0->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// chkClicked_c1
//

void
MainWindow::chkClicked_c1(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox c1 clicked");
  papp->checkboxClick(1, ui->chk1->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// chkClicked_c2
//

void
MainWindow::chkClicked_c2(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox c2 clicked");
  papp->checkboxClick(2, ui->chk2->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// chkClicked_c3
//

void
MainWindow::chkClicked_c3(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox c3 clicked");
  papp->checkboxClick(3, ui->chk3->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// chkClicked_c4
//

void
MainWindow::chkClicked_c4(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox c4 clicked");
  papp->checkboxClick(4, ui->chk4->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// chkClicked_c5
//

void
MainWindow::chkClicked_c5(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox c5 clicked");
  papp->checkboxClick(5, ui->chk5->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// chkClicked_c6
//

void
MainWindow::chkClicked_c6(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox c6 clicked");
  papp->checkboxClick(6, ui->chk6->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// chkClicked_c7
//

void
MainWindow::chkClicked_c7(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox c7 clicked");
  papp->checkboxClick(7, ui->chk7->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// chkClicked_c8
//

void
MainWindow::chkClicked_c8(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox c8 clicked");
  papp->checkboxClick(8, ui->chk8->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// chkClicked_c9
//

void
MainWindow::chkClicked_c9(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox c9 clicked");
  papp->checkboxClick(9, ui->chk9->isChecked());
}

//=============================================================================
//                              RADIO BUTTON
//=============================================================================

///////////////////////////////////////////////////////////////////////////////
// radioClicked_r0
//

void
MainWindow::radioClicked_r0(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox r0 clicked");
  papp->radioClick(0, ui->radio0->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// radioClicked_r1
//

void
MainWindow::radioClicked_r1(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox r1 clicked");
  papp->radioClick(1, ui->radio1->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// radioClicked_r2
//

void
MainWindow::radioClicked_r2(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox r2 clicked");
  papp->radioClick(2, ui->radio2->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// radioClicked_r3
//

void
MainWindow::radioClicked_r3(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox r3 clicked");
  papp->radioClick(3, ui->radio3->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// radioClicked_r4
//

void
MainWindow::radioClicked_r4(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox r4 clicked");
  papp->radioClick(4, ui->radio4->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// radioClicked_r5
//

void
MainWindow::radioClicked_r5(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox r5 clicked");
  papp->radioClick(5, ui->radio5->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// radioClicked_r6
//

void
MainWindow::radioClicked_r6(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox r6 clicked");
  papp->radioClick(6, ui->radio6->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// radioClicked_r7
//

void
MainWindow::radioClicked_r7(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox r7 clicked");
  papp->radioClick(7, ui->radio7->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// radioClicked_r8
//

void
MainWindow::radioClicked_r8(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox r8 clicked");
  papp->radioClick(8, ui->radio8->isChecked());
}

///////////////////////////////////////////////////////////////////////////////
// radioClicked_r9
//

void
MainWindow::radioClicked_r9(void)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();
  spdlog::info("Checkbox r9 clicked");
  papp->radioClick(9, ui->radio9->isChecked());
}

//=============================================================================
//                                 SLIDER
//=============================================================================

///////////////////////////////////////////////////////////////////////////////
// sliderChanged_s0
//

void
MainWindow::sliderChanged_s0(int value)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  spdlog::info("Checkbox s0 changed {}", value);
  papp->writeSliderValue(0, value);
}

///////////////////////////////////////////////////////////////////////////////
// sliderChanged_s1
//

void
MainWindow::sliderChanged_s1(int value)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  spdlog::info("Checkbox s1 changed {}", value);
  papp->writeSliderValue(1, value);
}

///////////////////////////////////////////////////////////////////////////////
// sliderChanged_s2
//

void
MainWindow::sliderChanged_s2(int value)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  spdlog::info("Checkbox s2 changed {}", value);
  papp->writeSliderValue(2, value);
}

///////////////////////////////////////////////////////////////////////////////
// sliderChanged_s3
//

void
MainWindow::sliderChanged_s3(int value)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  spdlog::info("Checkbox s3 changed {}", value);
  papp->writeSliderValue(3, value);
}

///////////////////////////////////////////////////////////////////////////////
// sliderChanged_s4
//

void
MainWindow::sliderChanged_s4(int value)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  spdlog::info("Checkbox s4 changed {}", value);
  papp->writeSliderValue(4, value);
}

///////////////////////////////////////////////////////////////////////////////
// sliderChanged_s5
//

void
MainWindow::sliderChanged_s5(int value)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  spdlog::info("Checkbox s5 changed {}", value);
  papp->writeSliderValue(5, value);
}

///////////////////////////////////////////////////////////////////////////////
// sliderChanged_s6
//

void
MainWindow::sliderChanged_s6(int value)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  spdlog::info("Checkbox s6 changed {}", value);
  papp->writeSliderValue(6, value);
}

///////////////////////////////////////////////////////////////////////////////
// sliderChanged_s7
//

void
MainWindow::sliderChanged_s7(int value)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  spdlog::info("Checkbox s7 changed {}", value);
  papp->writeSliderValue(7, value);
}

///////////////////////////////////////////////////////////////////////////////
// sliderChanged_s8
//

void
MainWindow::sliderChanged_s8(int value)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  spdlog::info("Checkbox s8 changed {}", value);
  papp->writeSliderValue(8, value);
}

///////////////////////////////////////////////////////////////////////////////
// sliderChanged_s9
//

void
MainWindow::sliderChanged_s9(int value)
{
  // Get pointer to app
  btest* papp = (btest*)QCoreApplication::instance();

  spdlog::info("Checkbox s9 changed {}", value);
  papp->writeSliderValue(9, value);
}
