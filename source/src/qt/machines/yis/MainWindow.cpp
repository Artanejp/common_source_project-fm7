/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for Yamaha YIS .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QtCore/QVariant>
#include <QtGui>
#include "menuclasses.h"
#include "commonclasses.h"

#include "emu.h"
#include "qt_main.h"
#include "vm.h"

//QT_BEGIN_NAMESPACE


void META_MainWindow::retranslateUi(void)
{
	retranslateControlMenu("", false);
	retranslateScreenMenu();
	//retranslateBinaryMenu(0, 1);
	retranslateCMTMenu(0);
	retranslateMachineMenu();
	retranslateUI_Help();
	retranslateEmulatorMenu();
	retranslateSoundMenu();
	
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
   // Set Labels
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
  
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
}


META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



