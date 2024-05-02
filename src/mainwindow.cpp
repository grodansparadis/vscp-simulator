#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtGlobal>

#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->lblQtVersion->setText(qVersion());
}

MainWindow::~MainWindow()
{
}
