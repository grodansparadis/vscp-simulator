#ifndef mainwindow_h
#define mainwindow_h

#include <QListWidgetItem>
#include <QMainWindow>
#include <QScopedPointer>
#include <QMessageBox>

#include <map>
#include <set>

#include <spdlog/async.h>
#include <spdlog/sinks/qt_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <vscp.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow(QWidget* parent = 0);
  virtual ~MainWindow();

  // Indexes for tabs
  const int tab_index_configuration = 0;
  const int tab_index_firmware      = 1;
  const int tab_index_register      = 2;
  const int tab_index_simulation    = 3;
  const int tab_index_logging       = 4;

  /*!
    Init application
  */
  void init(void);

  

  /*!
    Set user interface in firmware mode
  */
  void setFirmwareModeUi(void);

  /*!
    Set user interface in bootloader mode
  */
  void setBootLoaderModeUi(void);

  /*!
    Add register row
    Must be added in numerical order for both offset and page
    @param row Row number for register to update
    @param page Page for register to update
    @param value Value to write
  */
  //void addRegRow(uint32_t row, uint16_t page, uint8_t value);

  /*!
    Update register row
    @param row Row number for register to update
    @param page Page for register to update
    @param value Value to write
  */
  void updateRegRow(uint32_t row, uint16_t page, uint8_t value);

public slots:
  // Set checkbox value
  void setCheckboxValue(int idx, bool value);

  // Set slider value
  void setSliderValue(int idx, uint8_t value);

  // Set radio button value
  void setRadioButtonValue(int idx, bool value);

  // Set radio button value
  void setBackgroundColor(uint32_t color);

  /*!
    Initialize register
  */
  void initRegisters(void);

  /*!
    Update register
    @param offset for register (0-127 for level I)
    @param page for register (always zero for Level II)
    @param value Current value for register
  */
  void updateRegister(uint32_t offset, uint16_t page, uint8_t value);

  /*!
    Update window title (SIMULATION / BOOTLOADER)
    @param mode 0 = firmware, 0xff = bootloader
  */
  void updateWindowsTitle(int mode);

private slots:

  /*!
    Show about dialog
  */
  void about(void);

  void clearLog(void);
  void saveLog(void);
  void setLogLevel(int level);

  /*!
    Reset the simulated device
  */
  void resetDevice(void);

  /*!
    Set simulated device in boot mode
  */
  void setBootMode(void);

  /*!
    Set simulated device in firmware mode
  */
  void setFirmwareMode(void);

  void btnPressed_s0(void);
  void btnPressed_s1(void);
  void btnPressed_s2(void);
  void btnPressed_s3(void);
  void btnPressed_s4(void);
  void btnPressed_s5(void);
  void btnPressed_s6(void);
  void btnPressed_s7(void);
  void btnPressed_s8(void);
  void btnPressed_s9(void);

  void chkClicked_c0(void);
  void chkClicked_c1(void);
  void chkClicked_c2(void);
  void chkClicked_c3(void);
  void chkClicked_c4(void);
  void chkClicked_c5(void);
  void chkClicked_c6(void);
  void chkClicked_c7(void);
  void chkClicked_c8(void);
  void chkClicked_c9(void);

  void radioClicked_r0(void);
  void radioClicked_r1(void);
  void radioClicked_r2(void);
  void radioClicked_r3(void);
  void radioClicked_r4(void);
  void radioClicked_r5(void);
  void radioClicked_r6(void);
  void radioClicked_r7(void);
  void radioClicked_r8(void);
  void radioClicked_r9(void);

  void sliderChanged_s0(int value);
  void sliderChanged_s1(int value);
  void sliderChanged_s2(int value);
  void sliderChanged_s3(int value);
  void sliderChanged_s4(int value);
  void sliderChanged_s5(int value);
  void sliderChanged_s6(int value);
  void sliderChanged_s7(int value);
  void sliderChanged_s8(int value);
  void sliderChanged_s9(int value);

private:
  QScopedPointer<Ui::MainWindow> ui;

  // QT Log window
  std::shared_ptr<spdlog::sinks::qt_color_sink_mt> m_qt_sink;
};

#endif
