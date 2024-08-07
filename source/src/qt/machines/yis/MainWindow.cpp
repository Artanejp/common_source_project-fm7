/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for Yamaha YIS .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QtGui>
#include <QActionGroup>
#include <QMenu>

#include "menuclasses.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE


void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("", false);
   // Set Labels
#ifdef USE_MONITOR_TYPE
	actionMonitorType[0]->setText(QApplication::translate("MainWindow", "PU-1-10", 0));
	actionMonitorType[1]->setText(QApplication::translate("MainWindow", "PU-1-20", 0));
	actionMonitorType[2]->setText(QApplication::translate("MainWindow", "PU-10", 0));
#endif	
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
  
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



