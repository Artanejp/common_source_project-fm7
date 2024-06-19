/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for N5200 .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QtGui>
#include <QMenu>
#include <QActionGroup>

#include "menuclasses.h"
#include "qt_main.h"

void META_MainWindow::setupUI_Emu(void)
{
	menuMachine->setVisible(false);
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(" ",  false);
#ifdef USE_DEBUGGER
	actionDebugger[0]->setText(QApplication::translate("Machine", "Main CPU", 0));
	actionDebugger[0]->setVisible(true);
#endif
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
  
	actionReset->setText(QApplication::translate("MainWindow", "Reset", 0));
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



