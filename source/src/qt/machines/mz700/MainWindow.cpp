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
#include <QMenu>

#include "vm.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE

void META_MainWindow::setupUI_Emu(void)
{
#if !defined(_MZ800)
	//menuMachine->setVisible(false);
#endif   
#if defined(_MZ700)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(action_PCG700, 0x01,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(action_PCG700);
	menuMachine->addSeparator();
#endif
#if defined(USE_BOOT_MODE)
	ConfigCPUBootMode(USE_BOOT_MODE);
#endif

}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(" ",  true);
#if defined(USE_JOYSTICK_TYPE)
	actionJoystickType[0]->setText(QApplication::translate("MachineMZ700", "MZ-1X03", 0));
	actionJoystickType[0]->setToolTip(QApplication::translate("MachineMZ700", "Use Sharp MZ-1X03 Joystick unit.", 0));
	actionJoystickType[1]->setText(QApplication::translate("MachineMZ700", "JOY-700", 0));
	actionJoystickType[1]->setToolTip(QApplication::translate("MachineMZ700", "Use Tsukumo JOY-700 Joystick unit.", 0));
	actionJoystickType[2]->setText(QApplication::translate("MachineMZ700", "AM7J", 0));
	actionJoystickType[2]->setToolTip(QApplication::translate("MachineMZ700", "Use AM7J ATARI-Joystick adapter.", 0));
#endif
#if defined(_MZ800)
	menuBootMode->setTitle(QApplication::translate("Machine", "BOOT Mode", 0));
	actionBootMode[0]->setText(QString::fromUtf8("MZ-800"));
	actionBootMode[1]->setText(QString::fromUtf8("MZ-700"));
   
	menuMonitorType->setTitle("Monitor Type");
	menuMonitorType->setToolTipsVisible(true);
	actionMonitorType[0]->setText(QApplication::translate("MachineMZ700", "Color", 0));
	actionMonitorType[1]->setText(QApplication::translate("MachineMZ700", "Monochrome", 0));
	actionMonitorType[0]->setToolTip(QApplication::translate("MachineMZ700", "Use color CRT.", 0));
	actionMonitorType[1]->setToolTip(QApplication::translate("MachineMZ700", "Use monochrome CRT.", 0));
	menuMachine->setTitle(QApplication::translate("MachineMZ700", "Machine", 0));;

#elif defined(_MZ700)
	action_PCG700->setText(QApplication::translate("MachineMZ700", "PCG-700", 0));
	action_PCG700->setToolTip(QApplication::translate("MachineMZ700", "HAL laboratory PCG-700 PCG.", 0));
#endif
#if defined(_MZ1500)
	actionPrintDevice[1]->setText(QString::fromUtf8("MZ-1P17"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineMZ700", "Sharp MZ-1P17 kanji thermal printer.", 0));
	actionPrintDevice[2]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MenuMZ700", "NEC PC-PR201 kanji serial printer.", 0));
	actionPrintDevice[2]->setEnabled(false);
	
	actionPrintDevice[3]->setText(QString::fromUtf8("None"));
#endif
#if defined(USE_DRIVE_TYPE)
	menuDriveType->setTitle(QApplication::translate("MachineMZ700", "Floppy Type", 0));
	actionDriveType[0]->setText(QApplication::translate("MachineMZ700", "2D", 0));
	actionDriveType[1]->setText(QApplication::translate("MachineMZ700", "2DD", 0));
#endif
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
	// Set Labels
} // retranslateUi


META_MainWindow::META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



