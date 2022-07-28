/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QActionGroup>
#include <QMenu>
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"
#include "vm.h"



void META_MainWindow::setupUI_Emu(void)
{
	//menuMachine->setVisible(false);
#ifdef USE_CPU_TYPE
	ConfigCPUTypes(USE_CPU_TYPE);
#endif
 
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("Reset",  true);
	actionReset->setText(QApplication::translate("MachineMZ2500", "IPL Reset", 0));
	actionReset->setToolTip(QApplication::translate("MachineMZ2500", "Do IPL reset.", 0));
	actionSpecial_Reset[0]->setText(QApplication::translate("MachineMZ2500", "Reset", 0));
	actionSpecial_Reset[0]->setToolTip(QApplication::translate("MachineMZ2500", "Do system reset.", 0));
#ifdef USE_CPU_TYPE
	menuCpuType->setTitle(QApplication::translate("MachineMZ2500", "CPU Frequency", 0));
	actionCpuType[0]->setText(QString::fromUtf8("4MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("6MHz"));
#endif
#if defined(USE_DRIVE_TYPE)
	menuDriveType->setTitle(QApplication::translate("MachineMZ2500", "Floppy Type", 0));
	actionDriveType[0]->setText(QApplication::translate("MachineMZ2500", "2D", 0));
	actionDriveType[1]->setText(QApplication::translate("MachineMZ2500", "2DD", 0));
#endif

#if defined(USE_PRINTER)
#if defined(_MZ2500)
	actionPrintDevice[1]->setText(QString::fromUtf8("MZ-1P17"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineMZ2500", "Sharp MZ-1P17 Kanji thermal printer.", 0));
	actionPrintDevice[2]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MachineMZ2500", "NEC PC-PR201 Japanese serial printer.", 0));
	actionPrintDevice[2]->setEnabled(false);
#else
	actionPrintDevice[1]->setText(QString::fromUtf8("MZ-1P17 (MZ-1)"));
	actionPrintDevice[2]->setText(QString::fromUtf8("MZ-1P17 (MZ-3)"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineMZ2500", "Sharp MZ-1P17 thermal printer (MZ-1).", 0));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MachineMZ2500", "Sharp MZ-1P17 thermal printer (MZ-3).", 0));
#endif	
#endif
#if defined(USE_MONITOR_TYPE)
#if defined(_MZ2500)
	actionMonitorType[0]->setText(QApplication::translate("MachineMZ2500", "400Lines, Analog.", 0));
	actionMonitorType[1]->setText(QApplication::translate("MachineMZ2500", "400Lines, Digital.", 0));
	actionMonitorType[2]->setText(QApplication::translate("MachineMZ2500", "200Lines, Analog.", 0));
	actionMonitorType[3]->setText(QApplication::translate("MachineMZ2500", "200Lines, Digital.", 0));
#elif defined(_MZ2200)
	actionMonitorType[0]->setText(QApplication::translate("MachineMZ2500", "Color.", 0));
	actionMonitorType[1]->setText(QApplication::translate("MachineMZ2500", "Green.", 0));
	actionMonitorType[2]->setText(QApplication::translate("MachineMZ2500", "Both Color and Green.", 0));
	actionMonitorType[3]->setText(QApplication::translate("MachineMZ2500", "Both Green and Color.", 0));
#elif defined(_MZ80B)
	actionMonitorType[0]->setText(QApplication::translate("MachineMZ2500", "Green Monitor.", 0));
	actionMonitorType[1]->setText(QApplication::translate("MachineMZ2500", "Color Monitor (PIO-3039).", 0));
	actionMonitorType[2]->setText(QApplication::translate("MachineMZ2500", "Both Green and Color (PIO-3039).", 0));
	actionMonitorType[3]->setText(QApplication::translate("MachineMZ2500", "Both Color (PIO-3039) and Green.", 0));
#endif
#endif
#if defined(USE_DEBUGGER)
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#if defined(_MZ2200)
	actionDebugger[1]->setText(QString::fromUtf8("MZ-1M01 CPU"));
	actionDebugger[1]->setVisible(true);
#endif
#endif
	// Set Labels
} // retranslateUi



META_MainWindow::META_MainWindow(USING_FLAGS *p, std::shared_ptr<CSP_Logger> logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



