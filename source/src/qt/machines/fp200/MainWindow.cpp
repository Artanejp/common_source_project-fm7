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
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE




void META_MainWindow::retranslateUi(void)
{

	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("", false);
	menuBootMode->setTitle(QApplication::translate("Machine", "BOOT Mode", 0));
	menuBootMode->setToolTipsVisible(true);
	actionBootMode[0]->setText(QString::fromUtf8("BASIC"));
	actionBootMode[1]->setText(QString::fromUtf8("CETL"));	
	actionBootMode[0]->setToolTip(QApplication::translate("Machine", "Run as BASIC machine.", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("Machine", "Run as CETL (Spread-sheet language) machine.", 0));
	
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
	//	actionStart_Record_Movie->setText(QApplication::translate("Machine", "Start Record Movie", 0));
	//      actionStop_Record_Movie->setText(QApplication::translate("Machine", "Stop Record Movie", 0));
	// 
	// FP1100 Specified
	
	// Set Labels
	
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
	ConfigCPUBootMode(2);
}

META_MainWindow::META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



