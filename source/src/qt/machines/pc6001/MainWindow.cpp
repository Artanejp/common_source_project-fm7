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

#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

void META_MainWindow::retranslateUi(void)
{
	const char *title="";
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(title, false);
#ifdef USE_DEBUGGER
	actionDebugger[0]->setText(QApplication::translate("MenuPC6001", "Main CPU", 0));
	actionDebugger[1]->setText(QApplication::translate("MenuPC6001", "Sub  CPU", 0));
	actionDebugger[2]->setText(QApplication::translate("MenuPC6001", "PC-80S31K", 0));
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
	actionDebugger[2]->setVisible(true);
	actionDebugger[3]->setVisible(false);
#endif	
#if defined(USE_PRINTER)
	actionPrintDevice[1]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MenuPC6001", "NEC PC-PR201 kanji serial printer.", 0));
	actionPrintDevice[1]->setEnabled(false);
#endif

   // Set Labels
  
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{

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



