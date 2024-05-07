/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *formLayoutWidget;
    QFormLayout *formLayout;
    QTabWidget *tabWidget;
    QWidget *tab_configuration;
    QWidget *formLayoutWidget_2;
    QFormLayout *formLayout_2;
    QLabel *label;
    QLabel *lblQtVersion;
    QLabel *label_2;
    QLabel *lblInterface;
    QLabel *label_3;
    QLabel *lblConfiguration;
    QLabel *label_4;
    QLabel *lblHost;
    QLabel *label_5;
    QLabel *lblPort;
    QLabel *label_6;
    QLabel *lblUser;
    QLabel *label_7;
    QLabel *lblPassword;
    QLabel *label_8;
    QLabel *lblGuid;
    QWidget *tab_firmware;
    QWidget *verticalLayoutWidget_2;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_2;
    QToolButton *btnClearFirmware;
    QLabel *label_9;
    QLineEdit *lineEditCrc;
    QSpacerItem *horizontalSpacer_2;
    QTableView *tableViewFirmware;
    QWidget *tab_registers;
    QWidget *verticalLayoutWidget_3;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_3;
    QToolButton *toolButton;
    QToolButton *btnLoad;
    QSpacerItem *horizontalSpacer_3;
    QTreeView *treeView;
    QWidget *tab_sim;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QCheckBox *checkBox_8;
    QCheckBox *checkBox_7;
    QSlider *verticalSlider_9;
    QCheckBox *checkBox_6;
    QSlider *verticalSlider;
    QToolButton *btnSim0;
    QToolButton *btnSim1;
    QToolButton *btnSim3;
    QSlider *verticalSlider_2;
    QToolButton *btnSim8;
    QToolButton *btnSim2;
    QToolButton *btnSim5;
    QCheckBox *checkBox_5;
    QToolButton *btnSim7;
    QToolButton *btnSim4;
    QSlider *verticalSlider_10;
    QCheckBox *checkBox_9;
    QSlider *verticalSlider_7;
    QToolButton *btnSim9;
    QCheckBox *checkBox_10;
    QToolButton *btnSim6;
    QSpacerItem *horizontalSpacer_4;
    QCheckBox *checkBox_4;
    QSpacerItem *verticalSpacer;
    QCheckBox *checkBox_2;
    QCheckBox *checkBox;
    QSlider *verticalSlider_3;
    QSlider *verticalSlider_6;
    QSlider *verticalSlider_5;
    QSlider *verticalSlider_8;
    QCheckBox *checkBox_3;
    QSlider *verticalSlider_4;
    QRadioButton *radioButton_2;
    QRadioButton *radioButton_3;
    QRadioButton *radioButton_4;
    QRadioButton *radioButton_5;
    QRadioButton *radioButton_6;
    QRadioButton *radioButton_7;
    QRadioButton *radioButton_8;
    QRadioButton *radioButton_9;
    QRadioButton *radioButton_10;
    QRadioButton *radioButton;
    QWidget *tab_log;
    QWidget *verticalLayoutWidget;
    QVBoxLayout *verticalLayout;
    QHBoxLayout *horizontalLayout;
    QToolButton *btnClearLog;
    QToolButton *btnSave;
    QCheckBox *chkLog;
    QSpacerItem *horizontalSpacer;
    QTextEdit *txtEditLog;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1007, 752);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        formLayoutWidget = new QWidget(centralwidget);
        formLayoutWidget->setObjectName("formLayoutWidget");
        formLayoutWidget->setGeometry(QRect(9, 9, 991, 741));
        formLayout = new QFormLayout(formLayoutWidget);
        formLayout->setObjectName("formLayout");
        formLayout->setContentsMargins(0, 0, 0, 0);
        tabWidget = new QTabWidget(formLayoutWidget);
        tabWidget->setObjectName("tabWidget");
        tab_configuration = new QWidget();
        tab_configuration->setObjectName("tab_configuration");
        formLayoutWidget_2 = new QWidget(tab_configuration);
        formLayoutWidget_2->setObjectName("formLayoutWidget_2");
        formLayoutWidget_2->setGeometry(QRect(9, 9, 811, 641));
        formLayout_2 = new QFormLayout(formLayoutWidget_2);
        formLayout_2->setObjectName("formLayout_2");
        formLayout_2->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(formLayoutWidget_2);
        label->setObjectName("label");
        QFont font;
        font.setBold(true);
        label->setFont(font);
        label->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout_2->setWidget(0, QFormLayout::LabelRole, label);

        lblQtVersion = new QLabel(formLayoutWidget_2);
        lblQtVersion->setObjectName("lblQtVersion");

        formLayout_2->setWidget(0, QFormLayout::FieldRole, lblQtVersion);

        label_2 = new QLabel(formLayoutWidget_2);
        label_2->setObjectName("label_2");
        label_2->setFont(font);
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout_2->setWidget(1, QFormLayout::LabelRole, label_2);

        lblInterface = new QLabel(formLayoutWidget_2);
        lblInterface->setObjectName("lblInterface");

        formLayout_2->setWidget(1, QFormLayout::FieldRole, lblInterface);

        label_3 = new QLabel(formLayoutWidget_2);
        label_3->setObjectName("label_3");
        label_3->setFont(font);
        label_3->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout_2->setWidget(2, QFormLayout::LabelRole, label_3);

        lblConfiguration = new QLabel(formLayoutWidget_2);
        lblConfiguration->setObjectName("lblConfiguration");

        formLayout_2->setWidget(2, QFormLayout::FieldRole, lblConfiguration);

        label_4 = new QLabel(formLayoutWidget_2);
        label_4->setObjectName("label_4");
        label_4->setFont(font);
        label_4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout_2->setWidget(3, QFormLayout::LabelRole, label_4);

        lblHost = new QLabel(formLayoutWidget_2);
        lblHost->setObjectName("lblHost");

        formLayout_2->setWidget(3, QFormLayout::FieldRole, lblHost);

        label_5 = new QLabel(formLayoutWidget_2);
        label_5->setObjectName("label_5");
        label_5->setFont(font);
        label_5->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout_2->setWidget(4, QFormLayout::LabelRole, label_5);

        lblPort = new QLabel(formLayoutWidget_2);
        lblPort->setObjectName("lblPort");

        formLayout_2->setWidget(4, QFormLayout::FieldRole, lblPort);

        label_6 = new QLabel(formLayoutWidget_2);
        label_6->setObjectName("label_6");
        label_6->setFont(font);
        label_6->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout_2->setWidget(5, QFormLayout::LabelRole, label_6);

        lblUser = new QLabel(formLayoutWidget_2);
        lblUser->setObjectName("lblUser");

        formLayout_2->setWidget(5, QFormLayout::FieldRole, lblUser);

        label_7 = new QLabel(formLayoutWidget_2);
        label_7->setObjectName("label_7");
        label_7->setFont(font);
        label_7->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout_2->setWidget(6, QFormLayout::LabelRole, label_7);

        lblPassword = new QLabel(formLayoutWidget_2);
        lblPassword->setObjectName("lblPassword");

        formLayout_2->setWidget(6, QFormLayout::FieldRole, lblPassword);

        label_8 = new QLabel(formLayoutWidget_2);
        label_8->setObjectName("label_8");
        label_8->setFont(font);
        label_8->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        formLayout_2->setWidget(7, QFormLayout::LabelRole, label_8);

        lblGuid = new QLabel(formLayoutWidget_2);
        lblGuid->setObjectName("lblGuid");

        formLayout_2->setWidget(7, QFormLayout::FieldRole, lblGuid);

        tabWidget->addTab(tab_configuration, QString());
        tab_firmware = new QWidget();
        tab_firmware->setObjectName("tab_firmware");
        verticalLayoutWidget_2 = new QWidget(tab_firmware);
        verticalLayoutWidget_2->setObjectName("verticalLayoutWidget_2");
        verticalLayoutWidget_2->setGeometry(QRect(-1, 0, 841, 711));
        verticalLayout_2 = new QVBoxLayout(verticalLayoutWidget_2);
        verticalLayout_2->setObjectName("verticalLayout_2");
        verticalLayout_2->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        btnClearFirmware = new QToolButton(verticalLayoutWidget_2);
        btnClearFirmware->setObjectName("btnClearFirmware");

        horizontalLayout_2->addWidget(btnClearFirmware);

        label_9 = new QLabel(verticalLayoutWidget_2);
        label_9->setObjectName("label_9");
        label_9->setIndent(5);

        horizontalLayout_2->addWidget(label_9);

        lineEditCrc = new QLineEdit(verticalLayoutWidget_2);
        lineEditCrc->setObjectName("lineEditCrc");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lineEditCrc->sizePolicy().hasHeightForWidth());
        lineEditCrc->setSizePolicy(sizePolicy);
        lineEditCrc->setMaximumSize(QSize(70, 16777215));
        lineEditCrc->setReadOnly(true);

        horizontalLayout_2->addWidget(lineEditCrc);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        verticalLayout_2->addLayout(horizontalLayout_2);

        tableViewFirmware = new QTableView(verticalLayoutWidget_2);
        tableViewFirmware->setObjectName("tableViewFirmware");

        verticalLayout_2->addWidget(tableViewFirmware);

        tabWidget->addTab(tab_firmware, QString());
        tab_registers = new QWidget();
        tab_registers->setObjectName("tab_registers");
        verticalLayoutWidget_3 = new QWidget(tab_registers);
        verticalLayoutWidget_3->setObjectName("verticalLayoutWidget_3");
        verticalLayoutWidget_3->setGeometry(QRect(-1, 0, 841, 711));
        verticalLayout_3 = new QVBoxLayout(verticalLayoutWidget_3);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(0, 0, 0, 0);
        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        toolButton = new QToolButton(verticalLayoutWidget_3);
        toolButton->setObjectName("toolButton");

        horizontalLayout_3->addWidget(toolButton);

        btnLoad = new QToolButton(verticalLayoutWidget_3);
        btnLoad->setObjectName("btnLoad");

        horizontalLayout_3->addWidget(btnLoad);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);


        verticalLayout_3->addLayout(horizontalLayout_3);

        treeView = new QTreeView(verticalLayoutWidget_3);
        treeView->setObjectName("treeView");

        verticalLayout_3->addWidget(treeView);

        tabWidget->addTab(tab_registers, QString());
        tab_sim = new QWidget();
        tab_sim->setObjectName("tab_sim");
        gridLayoutWidget = new QWidget(tab_sim);
        gridLayoutWidget->setObjectName("gridLayoutWidget");
        gridLayoutWidget->setGeometry(QRect(-1, 0, 1172, 711));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName("gridLayout");
        gridLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
        gridLayout->setContentsMargins(0, 0, 0, 0);
        checkBox_8 = new QCheckBox(gridLayoutWidget);
        checkBox_8->setObjectName("checkBox_8");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(checkBox_8->sizePolicy().hasHeightForWidth());
        checkBox_8->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(checkBox_8, 1, 8, 1, 1);

        checkBox_7 = new QCheckBox(gridLayoutWidget);
        checkBox_7->setObjectName("checkBox_7");
        sizePolicy1.setHeightForWidth(checkBox_7->sizePolicy().hasHeightForWidth());
        checkBox_7->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(checkBox_7, 1, 7, 1, 1);

        verticalSlider_9 = new QSlider(gridLayoutWidget);
        verticalSlider_9->setObjectName("verticalSlider_9");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Expanding);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(verticalSlider_9->sizePolicy().hasHeightForWidth());
        verticalSlider_9->setSizePolicy(sizePolicy2);
        verticalSlider_9->setOrientation(Qt::Vertical);

        gridLayout->addWidget(verticalSlider_9, 2, 9, 1, 1);

        checkBox_6 = new QCheckBox(gridLayoutWidget);
        checkBox_6->setObjectName("checkBox_6");
        sizePolicy1.setHeightForWidth(checkBox_6->sizePolicy().hasHeightForWidth());
        checkBox_6->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(checkBox_6, 1, 6, 1, 1);

        verticalSlider = new QSlider(gridLayoutWidget);
        verticalSlider->setObjectName("verticalSlider");
        sizePolicy2.setHeightForWidth(verticalSlider->sizePolicy().hasHeightForWidth());
        verticalSlider->setSizePolicy(sizePolicy2);
        verticalSlider->setOrientation(Qt::Vertical);

        gridLayout->addWidget(verticalSlider, 2, 0, 1, 1);

        btnSim0 = new QToolButton(gridLayoutWidget);
        btnSim0->setObjectName("btnSim0");
        sizePolicy1.setHeightForWidth(btnSim0->sizePolicy().hasHeightForWidth());
        btnSim0->setSizePolicy(sizePolicy1);
        btnSim0->setMaximumSize(QSize(50, 16777215));
        btnSim0->setLayoutDirection(Qt::LeftToRight);

        gridLayout->addWidget(btnSim0, 0, 0, 1, 1);

        btnSim1 = new QToolButton(gridLayoutWidget);
        btnSim1->setObjectName("btnSim1");
        sizePolicy1.setHeightForWidth(btnSim1->sizePolicy().hasHeightForWidth());
        btnSim1->setSizePolicy(sizePolicy1);
        btnSim1->setMaximumSize(QSize(50, 16777215));

        gridLayout->addWidget(btnSim1, 0, 1, 1, 1);

        btnSim3 = new QToolButton(gridLayoutWidget);
        btnSim3->setObjectName("btnSim3");
        sizePolicy1.setHeightForWidth(btnSim3->sizePolicy().hasHeightForWidth());
        btnSim3->setSizePolicy(sizePolicy1);
        btnSim3->setMaximumSize(QSize(50, 16777215));

        gridLayout->addWidget(btnSim3, 0, 3, 1, 1);

        verticalSlider_2 = new QSlider(gridLayoutWidget);
        verticalSlider_2->setObjectName("verticalSlider_2");
        sizePolicy2.setHeightForWidth(verticalSlider_2->sizePolicy().hasHeightForWidth());
        verticalSlider_2->setSizePolicy(sizePolicy2);
        verticalSlider_2->setOrientation(Qt::Vertical);

        gridLayout->addWidget(verticalSlider_2, 2, 1, 1, 1);

        btnSim8 = new QToolButton(gridLayoutWidget);
        btnSim8->setObjectName("btnSim8");
        sizePolicy1.setHeightForWidth(btnSim8->sizePolicy().hasHeightForWidth());
        btnSim8->setSizePolicy(sizePolicy1);
        btnSim8->setMaximumSize(QSize(50, 16777215));

        gridLayout->addWidget(btnSim8, 0, 8, 1, 1);

        btnSim2 = new QToolButton(gridLayoutWidget);
        btnSim2->setObjectName("btnSim2");
        sizePolicy1.setHeightForWidth(btnSim2->sizePolicy().hasHeightForWidth());
        btnSim2->setSizePolicy(sizePolicy1);
        btnSim2->setMaximumSize(QSize(50, 16777215));

        gridLayout->addWidget(btnSim2, 0, 2, 1, 1);

        btnSim5 = new QToolButton(gridLayoutWidget);
        btnSim5->setObjectName("btnSim5");
        sizePolicy1.setHeightForWidth(btnSim5->sizePolicy().hasHeightForWidth());
        btnSim5->setSizePolicy(sizePolicy1);
        btnSim5->setMaximumSize(QSize(50, 16777215));

        gridLayout->addWidget(btnSim5, 0, 5, 1, 1);

        checkBox_5 = new QCheckBox(gridLayoutWidget);
        checkBox_5->setObjectName("checkBox_5");
        sizePolicy1.setHeightForWidth(checkBox_5->sizePolicy().hasHeightForWidth());
        checkBox_5->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(checkBox_5, 1, 5, 1, 1);

        btnSim7 = new QToolButton(gridLayoutWidget);
        btnSim7->setObjectName("btnSim7");
        sizePolicy1.setHeightForWidth(btnSim7->sizePolicy().hasHeightForWidth());
        btnSim7->setSizePolicy(sizePolicy1);
        btnSim7->setMaximumSize(QSize(50, 16777215));

        gridLayout->addWidget(btnSim7, 0, 7, 1, 1);

        btnSim4 = new QToolButton(gridLayoutWidget);
        btnSim4->setObjectName("btnSim4");
        sizePolicy1.setHeightForWidth(btnSim4->sizePolicy().hasHeightForWidth());
        btnSim4->setSizePolicy(sizePolicy1);
        btnSim4->setMaximumSize(QSize(50, 16777215));

        gridLayout->addWidget(btnSim4, 0, 4, 1, 1);

        verticalSlider_10 = new QSlider(gridLayoutWidget);
        verticalSlider_10->setObjectName("verticalSlider_10");
        sizePolicy2.setHeightForWidth(verticalSlider_10->sizePolicy().hasHeightForWidth());
        verticalSlider_10->setSizePolicy(sizePolicy2);
        verticalSlider_10->setOrientation(Qt::Vertical);

        gridLayout->addWidget(verticalSlider_10, 2, 7, 1, 1);

        checkBox_9 = new QCheckBox(gridLayoutWidget);
        checkBox_9->setObjectName("checkBox_9");
        sizePolicy1.setHeightForWidth(checkBox_9->sizePolicy().hasHeightForWidth());
        checkBox_9->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(checkBox_9, 1, 0, 1, 1);

        verticalSlider_7 = new QSlider(gridLayoutWidget);
        verticalSlider_7->setObjectName("verticalSlider_7");
        sizePolicy2.setHeightForWidth(verticalSlider_7->sizePolicy().hasHeightForWidth());
        verticalSlider_7->setSizePolicy(sizePolicy2);
        verticalSlider_7->setOrientation(Qt::Vertical);

        gridLayout->addWidget(verticalSlider_7, 2, 6, 1, 1);

        btnSim9 = new QToolButton(gridLayoutWidget);
        btnSim9->setObjectName("btnSim9");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(btnSim9->sizePolicy().hasHeightForWidth());
        btnSim9->setSizePolicy(sizePolicy3);
        btnSim9->setMaximumSize(QSize(50, 16777215));

        gridLayout->addWidget(btnSim9, 0, 9, 1, 1);

        checkBox_10 = new QCheckBox(gridLayoutWidget);
        checkBox_10->setObjectName("checkBox_10");
        sizePolicy1.setHeightForWidth(checkBox_10->sizePolicy().hasHeightForWidth());
        checkBox_10->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(checkBox_10, 1, 9, 1, 1);

        btnSim6 = new QToolButton(gridLayoutWidget);
        btnSim6->setObjectName("btnSim6");
        sizePolicy1.setHeightForWidth(btnSim6->sizePolicy().hasHeightForWidth());
        btnSim6->setSizePolicy(sizePolicy1);
        btnSim6->setMaximumSize(QSize(50, 16777215));

        gridLayout->addWidget(btnSim6, 0, 6, 1, 1);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        gridLayout->addItem(horizontalSpacer_4, 0, 11, 1, 1);

        checkBox_4 = new QCheckBox(gridLayoutWidget);
        checkBox_4->setObjectName("checkBox_4");
        sizePolicy1.setHeightForWidth(checkBox_4->sizePolicy().hasHeightForWidth());
        checkBox_4->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(checkBox_4, 1, 4, 1, 1);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        gridLayout->addItem(verticalSpacer, 4, 1, 1, 1);

        checkBox_2 = new QCheckBox(gridLayoutWidget);
        checkBox_2->setObjectName("checkBox_2");
        sizePolicy1.setHeightForWidth(checkBox_2->sizePolicy().hasHeightForWidth());
        checkBox_2->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(checkBox_2, 1, 2, 1, 1);

        checkBox = new QCheckBox(gridLayoutWidget);
        checkBox->setObjectName("checkBox");
        sizePolicy1.setHeightForWidth(checkBox->sizePolicy().hasHeightForWidth());
        checkBox->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(checkBox, 1, 1, 1, 1);

        verticalSlider_3 = new QSlider(gridLayoutWidget);
        verticalSlider_3->setObjectName("verticalSlider_3");
        sizePolicy2.setHeightForWidth(verticalSlider_3->sizePolicy().hasHeightForWidth());
        verticalSlider_3->setSizePolicy(sizePolicy2);
        verticalSlider_3->setOrientation(Qt::Vertical);

        gridLayout->addWidget(verticalSlider_3, 2, 2, 1, 1);

        verticalSlider_6 = new QSlider(gridLayoutWidget);
        verticalSlider_6->setObjectName("verticalSlider_6");
        sizePolicy2.setHeightForWidth(verticalSlider_6->sizePolicy().hasHeightForWidth());
        verticalSlider_6->setSizePolicy(sizePolicy2);
        verticalSlider_6->setOrientation(Qt::Vertical);

        gridLayout->addWidget(verticalSlider_6, 2, 5, 1, 1);

        verticalSlider_5 = new QSlider(gridLayoutWidget);
        verticalSlider_5->setObjectName("verticalSlider_5");
        sizePolicy2.setHeightForWidth(verticalSlider_5->sizePolicy().hasHeightForWidth());
        verticalSlider_5->setSizePolicy(sizePolicy2);
        verticalSlider_5->setOrientation(Qt::Vertical);

        gridLayout->addWidget(verticalSlider_5, 2, 4, 1, 1);

        verticalSlider_8 = new QSlider(gridLayoutWidget);
        verticalSlider_8->setObjectName("verticalSlider_8");
        sizePolicy2.setHeightForWidth(verticalSlider_8->sizePolicy().hasHeightForWidth());
        verticalSlider_8->setSizePolicy(sizePolicy2);
        verticalSlider_8->setOrientation(Qt::Vertical);

        gridLayout->addWidget(verticalSlider_8, 2, 8, 1, 1);

        checkBox_3 = new QCheckBox(gridLayoutWidget);
        checkBox_3->setObjectName("checkBox_3");
        sizePolicy1.setHeightForWidth(checkBox_3->sizePolicy().hasHeightForWidth());
        checkBox_3->setSizePolicy(sizePolicy1);

        gridLayout->addWidget(checkBox_3, 1, 3, 1, 1);

        verticalSlider_4 = new QSlider(gridLayoutWidget);
        verticalSlider_4->setObjectName("verticalSlider_4");
        sizePolicy2.setHeightForWidth(verticalSlider_4->sizePolicy().hasHeightForWidth());
        verticalSlider_4->setSizePolicy(sizePolicy2);
        verticalSlider_4->setOrientation(Qt::Vertical);

        gridLayout->addWidget(verticalSlider_4, 2, 3, 1, 1);

        radioButton_2 = new QRadioButton(gridLayoutWidget);
        radioButton_2->setObjectName("radioButton_2");

        gridLayout->addWidget(radioButton_2, 3, 1, 1, 1);

        radioButton_3 = new QRadioButton(gridLayoutWidget);
        radioButton_3->setObjectName("radioButton_3");

        gridLayout->addWidget(radioButton_3, 3, 2, 1, 1);

        radioButton_4 = new QRadioButton(gridLayoutWidget);
        radioButton_4->setObjectName("radioButton_4");

        gridLayout->addWidget(radioButton_4, 3, 9, 1, 1);

        radioButton_5 = new QRadioButton(gridLayoutWidget);
        radioButton_5->setObjectName("radioButton_5");

        gridLayout->addWidget(radioButton_5, 3, 8, 1, 1);

        radioButton_6 = new QRadioButton(gridLayoutWidget);
        radioButton_6->setObjectName("radioButton_6");

        gridLayout->addWidget(radioButton_6, 3, 7, 1, 1);

        radioButton_7 = new QRadioButton(gridLayoutWidget);
        radioButton_7->setObjectName("radioButton_7");

        gridLayout->addWidget(radioButton_7, 3, 6, 1, 1);

        radioButton_8 = new QRadioButton(gridLayoutWidget);
        radioButton_8->setObjectName("radioButton_8");

        gridLayout->addWidget(radioButton_8, 3, 5, 1, 1);

        radioButton_9 = new QRadioButton(gridLayoutWidget);
        radioButton_9->setObjectName("radioButton_9");

        gridLayout->addWidget(radioButton_9, 3, 4, 1, 1);

        radioButton_10 = new QRadioButton(gridLayoutWidget);
        radioButton_10->setObjectName("radioButton_10");

        gridLayout->addWidget(radioButton_10, 3, 3, 1, 1);

        radioButton = new QRadioButton(gridLayoutWidget);
        radioButton->setObjectName("radioButton");

        gridLayout->addWidget(radioButton, 3, 0, 1, 1);

        tabWidget->addTab(tab_sim, QString());
        tab_log = new QWidget();
        tab_log->setObjectName("tab_log");
        verticalLayoutWidget = new QWidget(tab_log);
        verticalLayoutWidget->setObjectName("verticalLayoutWidget");
        verticalLayoutWidget->setGeometry(QRect(-1, 0, 831, 711));
        verticalLayout = new QVBoxLayout(verticalLayoutWidget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName("horizontalLayout");
        btnClearLog = new QToolButton(verticalLayoutWidget);
        btnClearLog->setObjectName("btnClearLog");

        horizontalLayout->addWidget(btnClearLog);

        btnSave = new QToolButton(verticalLayoutWidget);
        btnSave->setObjectName("btnSave");

        horizontalLayout->addWidget(btnSave);

        chkLog = new QCheckBox(verticalLayoutWidget);
        chkLog->setObjectName("chkLog");
        chkLog->setChecked(true);

        horizontalLayout->addWidget(chkLog);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);

        txtEditLog = new QTextEdit(verticalLayoutWidget);
        txtEditLog->setObjectName("txtEditLog");

        verticalLayout->addWidget(txtEditLog);

        tabWidget->addTab(tab_log, QString());

        formLayout->setWidget(0, QFormLayout::SpanningRole, tabWidget);

        MainWindow->setCentralWidget(centralwidget);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(4);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "VSCP Bootloader test app.", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Qt Version: ", nullptr));
        lblQtVersion->setText(QCoreApplication::translate("MainWindow", "----------------------------", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Interface:", nullptr));
        lblInterface->setText(QCoreApplication::translate("MainWindow", "----------------------------", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Configuration:", nullptr));
        lblConfiguration->setText(QCoreApplication::translate("MainWindow", "----------------------------", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "Host:", nullptr));
        lblHost->setText(QCoreApplication::translate("MainWindow", "----------------------------", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "Port:", nullptr));
        lblPort->setText(QCoreApplication::translate("MainWindow", "----------------------------", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "User:", nullptr));
        lblUser->setText(QCoreApplication::translate("MainWindow", "----------------------------", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "Password:", nullptr));
        lblPassword->setText(QCoreApplication::translate("MainWindow", "----------------------------", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "GUID:", nullptr));
        lblGuid->setText(QCoreApplication::translate("MainWindow", "----------------------------", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_configuration), QCoreApplication::translate("MainWindow", "Configuration", nullptr));
        btnClearFirmware->setText(QCoreApplication::translate("MainWindow", "Clear", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "CRC:", nullptr));
        lineEditCrc->setText(QCoreApplication::translate("MainWindow", "0x1AFF", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_firmware), QCoreApplication::translate("MainWindow", "Firmware", nullptr));
        toolButton->setText(QCoreApplication::translate("MainWindow", "Clear", nullptr));
        btnLoad->setText(QCoreApplication::translate("MainWindow", "Load", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_registers), QCoreApplication::translate("MainWindow", "Registers", nullptr));
        checkBox_8->setText(QCoreApplication::translate("MainWindow", "C8", nullptr));
        checkBox_7->setText(QCoreApplication::translate("MainWindow", "C7", nullptr));
        checkBox_6->setText(QCoreApplication::translate("MainWindow", "C6", nullptr));
        btnSim0->setText(QCoreApplication::translate("MainWindow", "S0", nullptr));
        btnSim1->setText(QCoreApplication::translate("MainWindow", "S1", nullptr));
        btnSim3->setText(QCoreApplication::translate("MainWindow", "S3", nullptr));
        btnSim8->setText(QCoreApplication::translate("MainWindow", "S8", nullptr));
        btnSim2->setText(QCoreApplication::translate("MainWindow", "S2", nullptr));
        btnSim5->setText(QCoreApplication::translate("MainWindow", "S5", nullptr));
        checkBox_5->setText(QCoreApplication::translate("MainWindow", "C5", nullptr));
        btnSim7->setText(QCoreApplication::translate("MainWindow", "S7", nullptr));
        btnSim4->setText(QCoreApplication::translate("MainWindow", "S4", nullptr));
        checkBox_9->setText(QCoreApplication::translate("MainWindow", "C0", nullptr));
        btnSim9->setText(QCoreApplication::translate("MainWindow", "S9", nullptr));
        checkBox_10->setText(QCoreApplication::translate("MainWindow", "C9", nullptr));
        btnSim6->setText(QCoreApplication::translate("MainWindow", "S6", nullptr));
        checkBox_4->setText(QCoreApplication::translate("MainWindow", "C4", nullptr));
        checkBox_2->setText(QCoreApplication::translate("MainWindow", "C2", nullptr));
        checkBox->setText(QCoreApplication::translate("MainWindow", "C1", nullptr));
        checkBox_3->setText(QCoreApplication::translate("MainWindow", "C3", nullptr));
        radioButton_2->setText(QCoreApplication::translate("MainWindow", "R1", nullptr));
        radioButton_3->setText(QCoreApplication::translate("MainWindow", "R2", nullptr));
        radioButton_4->setText(QCoreApplication::translate("MainWindow", "R9", nullptr));
        radioButton_5->setText(QCoreApplication::translate("MainWindow", "R8", nullptr));
        radioButton_6->setText(QCoreApplication::translate("MainWindow", "R7", nullptr));
        radioButton_7->setText(QCoreApplication::translate("MainWindow", "R6", nullptr));
        radioButton_8->setText(QCoreApplication::translate("MainWindow", "R5", nullptr));
        radioButton_9->setText(QCoreApplication::translate("MainWindow", "R4", nullptr));
        radioButton_10->setText(QCoreApplication::translate("MainWindow", "R3", nullptr));
        radioButton->setText(QCoreApplication::translate("MainWindow", "R0", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_sim), QCoreApplication::translate("MainWindow", "Sim", nullptr));
        btnClearLog->setText(QCoreApplication::translate("MainWindow", "Clear", nullptr));
        btnSave->setText(QCoreApplication::translate("MainWindow", "Save", nullptr));
        chkLog->setText(QCoreApplication::translate("MainWindow", "Active", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_log), QCoreApplication::translate("MainWindow", "Log", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
