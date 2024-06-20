#ifndef mainwindow_h
#define mainwindow_h

#include <QMainWindow>
#include <QScopedPointer>

#include <spdlog/async.h>
#include <spdlog/sinks/qt_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

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

public slots:
  // Set checkbox value
  void setCheckboxValue(int idx, bool value);

  // Set slider value
  void setSliderValue(int idx, uint8_t value);

  // Set radio button value
  void setRadioButtonValue(int idx, bool value);

  // Set radio button value
  void setBackgroundColor(uint32_t color);

private slots:
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

private:
  QScopedPointer<Ui::MainWindow> ui;

  // QT Log window
  std::shared_ptr<spdlog::sinks::qt_color_sink_mt> m_qt_sink;
};

#endif
