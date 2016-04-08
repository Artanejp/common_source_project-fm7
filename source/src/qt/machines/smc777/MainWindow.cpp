/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for SMC777/SMC70 .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QVariant>
#include <QtGui>
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE


void META_MainWindow::setupUI_Emu(void)
{
#if defined(_SMC70)
	menuBootMode = new QMenu(menuMachine);
	menuBootMode->setObjectName(QString::fromUtf8("menuControl_BootMode"));
	menuMachine->addAction(menuBootMode->menuAction());
	ConfigCPUBootMode(3);
#endif
}

void META_MainWindow::retranslateUi(void)
{
	retranslateControlMenu("RESET",  true);
	actionReset->setVisible(false);
	retranslateFloppyMenu(0, 1);
	retranslateFloppyMenu(1, 2);
 
	retranslateCMTMenu();
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
#if defined(_SMC70)
	menuBootMode->setTitle(QApplication::translate("MainWindow", "Auto Start SW:", 0));
	actionBootMode[0]->setText(QApplication::translate("MainWindow", "ROM", 0));
	actionBootMode[1]->setText(QApplication::translate("MainWindow", "Disk", 0));
	actionBootMode[2]->setText(QApplication::translate("MainWindow", "Off", 0));
#endif	
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
  
  
	actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));
  

	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));
  
	menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0));
	actionHelp_AboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0));
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
   // Set Labels
} // retranslateUi



META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



