/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for Babbage2nd .
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
#include "../../vm/ys6464a/ys6464a.h"

//QT_BEGIN_NAMESPACE


void META_MainWindow::retranslateUi(void)
{
	retranslateControlMenu("", false);
	retranslateScreenMenu();
	retranslateBinaryMenu(0, 1);
	//retranslateSoundMenu();
	
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
	actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0));
	
	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));
   
	actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));
	menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0));
	menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0));  
	menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0));
	actionHelp_AboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0));
   // Set Labels
  
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
}


META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



