/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for SMC777/SMC70 .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QMenu>
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE


void META_MainWindow::setupUI_Emu(void)
{
#if defined(_SMC70)
	ConfigCPUBootMode(3);
#endif
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("RESET",  true);
	actionReset->setVisible(false);
	actionSpecial_Reset[0]->setToolTip(QApplication::translate("MainWindow", "RESET", 0));
	actionSpecial_Reset[0]->setToolTip(QApplication::translate("MainWindow", "Do reset.", 0));
	
#if defined(_SMC70)
	menuBootMode->setTitle(QApplication::translate("MainWindow", "Auto Start SW:", 0));
	menuBootMode->setToolTipsVisible(true);
	actionBootMode[0]->setText(QApplication::translate("MainWindow", "ROM", 0));
	actionBootMode[1]->setText(QApplication::translate("MainWindow", "Disk", 0));
	actionBootMode[2]->setText(QApplication::translate("MainWindow", "Off", 0));
#endif	
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
   // Set Labels
} // retranslateUi



META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



