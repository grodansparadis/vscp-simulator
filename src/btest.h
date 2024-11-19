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

#ifndef ___VSCP_BTEST_H
#define ___VSCP_BTEST_H

#include <vscp.h>

#include <guid.h>
#include <mdf.h>
#include <register.h>
#include <version.h>
#include <vscp-client-base.h>
#include <vscpunit.h>

#include <vscp-bootloader.h>
#include <vscp-firmware-level2.h>

#include "mainwindow.h"

#include <QApplication>
#include <QByteArray>
#include <QDateTime>
#include <QMainWindow>
#include <QMutex>
#include <QObject>
#include <QQueue>

#include <QSemaphore>
#include <list>

#include <pthread.h>

#include <mustache.hpp>
#include <nlohmann/json.hpp>

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/spdlog.h"

#include "simulation1.h"

// https://github.com/nlohmann/json
using json = nlohmann::json;

using namespace kainjow::mustache;

// Register to be used for signals
Q_DECLARE_METATYPE(vscpEvent)
Q_DECLARE_METATYPE(vscpEventEx)

/// Application name used in titles and headers
#define APPNAME "VSCP Simulator"

#define VSCPSIM_VERSION_MAJOR   0
#define VSCPSIM_VERSION_MINOR   0
#define VSCPSIM_VERSION_RELEASE 1

#define VSCPSIM_DISPLAY_VERSION "0.0.1 alpha"
#define VSCPSIM_COPYRIGHT                                   \
  "Copyright © 2000-2024 Ake Hedman, Grodans Paradis AB, " \
  "https://www.grodansparadis.com"
#define VSCPSIM_COPYRIGHT_HTML                                            \
  "Copyright © 2000-2024 Åke Hedman, <a "                               \
  "href=\"mailto:info@grodansparadis.com\">Grodans Paradis AB</a><br><a " \
  "href=\"https://www.grodansparadis.com\">https://"                      \
  "www.grodansparadis.com</a>"

#define VSCPSIM_VERSION(major, minor, release) \
  (((major) << 16) | ((minor) << 8) | (release))

// home folder is used for storage of program configuration
// system folder holds databases etc
#ifdef WIN32
#define DEFAULT_HOME_FOLDER        "c:/programdata/vscp/btest/"
#define DEFAULT_VSCP_SYSTEM_FOLDER "c:/programdata/vscp/"
#else
#define DEFAULT_HOME_FOLDER        "~/.btest/"
#define DEFAULT_VSCP_SYSTEM_FOLDER "/var/lib/vscp/"
#endif

#define DEFAULT_CONNECT_TIMOUT 5000

/*!
    Encapsulates VSCP works main settings
*/
class btest : public QApplication {

  Q_OBJECT

public:
  /*!
      btest
  */
  btest(int& argc, char** argv);

  /*!
      Destructor
  */
  ~btest();

  /*!
    Codes for mode we are in
  */
  enum mode {
    FIRMWARE   = 0,
    BOOTLOADER = 0xff,
  };

  /*!
    Run the workerthread (simulation)
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int startWorkerThread(void);

  /*!
    Stop the workerthread (simulation)
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int stopWorkerThread(void);

  /*!
      Load configuration settings from disk
  */
  void loadProgramSettings(void);

  /*!
      Save configuration settings to disk
  */
  void writeProgramSettings(void);

  /*!
    Load file for firmware configuration
    @param path Path to configuration path
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int loadFirmwareConfig(QString& path);

  /*!
    Get min window
    @return Pointer to main window
  */
  MainWindow* getMainWindow(void);

  void receiveCallback(vscpEventEx& ex, void* pobj);

  /*!
    Get event ex from input queue

    @param pex Pointer to received event or NULL if no event
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int getEventEx(vscpEventEx** pex);

  // ========================================================================
  //                        Simulation handlers
  // ========================================================================

  /*!
    Initialize simulation data
    @return True on success, false on failure
  */
  bool initSimulationData(void);

  /*!
    Read register for simulation 1
    @param page Page to read from
    @param reg Register to read from
    @param pval Pointer to value that will get result
    @return VSCP_ERROR_SUCCESS if all is OK. VSCP_ERROR_INDEX_OOB is returned when trying
    to read a register that does not exist. This error is actually a warning.
  */
  int readRegister_sim1(uint16_t page, uint32_t reg, uint8_t* pval);

  /*!
    Write register for simulation 1
    @param page Page to write to
    @param reg Register to write to
    @param val value that should be written
    @return VSCP_ERROR_SUCCESS if all is OK. VSCP_ERROR_INDEX_OOB is returned when trying
    to read a register that does not exist. This error is actually a warning.
  */
  int writeRegister_sim1(uint16_t page, uint32_t reg, uint8_t val);

  /*!
    report DM content
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int reportDM_sim1(void);

  /*!
    Slider has moved by user.
    This method distribute to a handler for the active simulation
    @param idx Slider idex 0-9
    @param value Current value of slider
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int writeSliderValue(uint8_t idx, int value);

  /*!
    Slider has moved by user and smulation handle it.
    @param idx Slider index 0-9
    @param value Current value of slider
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int writeSliderValue_sim1(uint8_t idx, int value);

  /*!
    Button press has occured in sim interface
    @param idx Slider index 0-9
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int buttonPress(int idx);

  /*!
    Button press handler for sim1
    @param idx Slider index 0-9
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int buttonPress_sim1(int idx);

  /*!
    Checkbox clicked in sim interface
    @param idx Checkbox index 0-9
    @param checked True if checked, false if unchecked
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int checkboxClick(int idx, bool checked);

  /*!
    Simulation handler for checkbox clicked
    @param idx Checkbox index 0-9
    @param checked True if checked, false if unchecked
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int checkboxClick_sim1(int idx, bool checked);

  /*!
    Radio button clicked in sim interface
    @param idx Radiobutton index 0-9
    @param checked True if checked, false if unchecked
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int radioClick(int idx, bool checked);

  /*!
    Simulation handler for radiobutton clicked
    @param idx Checkbox index 0-9
    @param checked True if checked, false if unchecked
    @return VSCP_ERROR_SUCCESS if all is OK
  */
  int radioClick_sim1(int idx, bool checked);

  // ========================================================================
  //                         Firmware callbacks
  // ========================================================================

  /*!
    Read register content
    @param page Page to read content from.
    @param reg Register to read.
    @param pval Pointer to variable that will get register content on success.
    @return VSCP_ERROR_SUCCESS on OK. VSCP_ERROR_INDEX_OOB is returned when trying
    to read a register that does not exist. This error is actually a warning.
  */
  int readRegister(uint16_t page, uint32_t reg, uint8_t* pval);

  /*!
    Write register content
    @param page Page to read content from.
    @param reg Regsiter to read.
    @param val Value to write to register.
    @return VSCP_ERROR_SUCCESS on OK. VSCP_ERROR_INDEX_OOB is returned when trying
    to read a register that does not exist. This error is actually a warning.
  */
  int writeRegister(uint16_t page, uint32_t reg, uint8_t val);

  int receivedSegCtrlHeartBeat(uint16_t segcrc, uint32_t tm);

  int newNodeOnline(uint16_t nickname);

  int newNodeOnline(cguid& guid);

  int reportDM(void);

  int reportEventsOfInterest(void);

  int standardRegHasChanged(uint32_t stdreg);

  /*!
    Get URL for MDF (max 43 characters in length)
  */
  int getMdfUrl(uint8_t* const purl);

  /*!
    Send embedded MDF if one is available
  */
  int sendEmbeddedMDF(void);

  // ========================================================================
  //                         Bootloader callbacks
  // ========================================================================

  void vscpboot_goApplication(void);

  int vscpboot_init_hardware(void);

  int vscpboot_release_hardware(void);

  uint8_t vscpboot_getBootFlag(void);

  int vscpboot_setBootFlag(uint8_t bootflag);

  void vscpboot_reboot(void);

  vscpboot_config_t* vscpboot_getConfig(void);

  uint8_t* vscpboot_getGUID(void);

  int vscpboot_isMemTypeValid(uint8_t type);

  int vscpboot_isMemBankValid(uint8_t bank);

  int vscpboot_programBlock(const uint8_t* pblock, uint8_t type, uint8_t bank);

  crc vscpboot_calcPrgCrc(void);

  int vscpboot_sendEventEx(vscpEventEx* pex);

  int vscpboot_getEventEx(vscpEventEx* pex);

  // ========================================================================
  // ========================================================================

  // ------------------------------------------------------------------------
  // Global Configuration information below
  //   This info is read from a configuration file
  //   at startup and saved on close. The configuration
  //   file should be placed in the home folder.
  // ------------------------------------------------------------------------

  // ------------------------------------------------------------------------

  /*!
    Flag for workerthread execution
    Set to false to end execution of thread
  */
  bool m_bRun;

  /*!
    The bootflag controls what code that is executed
    it is zero if the appcication code should be executed
    and non-zero if the bootloader should be started.
  */
  uint8_t m_bootflag;

  // Path to configuration file
  QString m_configpath;

  /*!
    Bootloader configuration
  */
  vscpboot_config_t m_bootloader_cfg;

  /*!
    Firmware configuration
  */
  vscp_frmw2_firmware_config_t m_firmware_cfg;

  /*!
    Worker thread
    This thread do the actual work. It can be a
    device in bootmode or a running firmware device
    depending on the value of the bootflag.
  */
  pthread_t m_threadWork;

  /// Folder used for configuration
  /// Linux: ~/.configure/VSCP/(btest.conf)
  QString m_configFolder;

  /// Folder for writeable data
  /// Linux: ~/.local/share/vscp/btest
  QString m_shareFolder;

  // Folder used for VSCP files like db's
  // Linux:
  // vscp/drivers/level1 - contain level one drivers
  // vscp/drivers/level2 - contain level two drivers
  // Windows:
  // c:/program data/vscp/drivers/level1
  // c:/program data/vscp/drivers/level2
  QString m_vscpHomeFolder;

  /*!
   Active simulation
  */
  uint8_t m_nSimulation;

  /// Pointer to active simulation structure
  void* m_pSim;

  /*!
    The communication interface
  */
  CVscpClient* m_pClient;

  /// Timeout for connect in milliseconds
  uint32_t m_timeoutConnect;

  /// protects receive queue
  sem_t m_semReceiveQueue;
  pthread_mutex_t m_mutexReceiveQueue;

  /*!
    Queue holding received events
  */
  QQueue<vscpEventEx*> m_inqueue;

  // ---------------------------------------------------

  /// Interface
  QString m_interface;

  /// Configuration String
  QString m_config;

  std::deque<std::string> m_configVector;

  /// GUID
  // cguid m_guid;

  /// Host
  QString m_host;

  /// Port
  uint16_t m_port;

  /// User
  QString m_user;

  /// Password
  QString m_password;

  //**************************************************************************
  //                            LOGGER (SPDLOG)
  //**************************************************************************

  bool m_bEnableFileLog;
  spdlog::level::level_enum m_fileLogLevel;
  std::string m_fileLogPattern;
  std::string m_fileLogPath;
  uint32_t m_maxFileLogSize;
  uint16_t m_maxFileLogFiles;

  bool m_bEnableConsoleLog;
  spdlog::level::level_enum m_consoleLogLevel;
  std::string m_consoleLogPattern;

  // Maps (page + register) to pointer to widgetitem
  std::map<uint32_t, QListWidgetItem*> m_regmap;

  // Initial register values, but only ones != 0
  std::set<uint8_t> m_initial_value;

signals:

  /// Data received from callback
  void dataReceived(vscpEventEx* pex);

  /// Simulation checkbox value changed
  void checkValueChanged(int idx, bool value);

  /// Set slider value of slider in mainwindow
  void sliderValueChanged(int idx, uint8_t value);

  /// Simulation radio button value changed
  void radioValueChanged(int idx, bool value);

  /// Background color changed
  void backgroundColorChanged(uint32_t color);

  /// Init registers  (VSCP_LEVEL1)
  void initRegisters(std::set<uint32_t>& regset, uint8_t level);

  /// Update register list
  void updateRegister(uint32_t offset, uint16_t page, uint8_t value);
};

#endif // ___VSCP_BTEST_H
