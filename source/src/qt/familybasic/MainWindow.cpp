/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for JR100 .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QVariant>
#include <QtGui>
#include "emu.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "qt_main.h"


void META_MainWindow::setupUI_Emu(void)
{
	menuBootMode = new QMenu(menuMachine);
	menuBootMode->setObjectName(QString::fromUtf8("menuControl_BootMode"));
	menuMachine->addAction(menuBootMode->menuAction());
	ConfigCPUBootMode(3);
}

void META_MainWindow::retranslateUi(void)
{
	int i;
	retranslateControlMenu("System Reset",  false);
	retranslateCMTMenu();
	retranslateSoundMenu();
	retranslateScreenMenu();
   
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
  
	actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));

	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));
  
	menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0));
	actionHelp_AboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0));

	menuBootMode->setTitle("BOOT Mode");
	actionBootMode[0]->setText(QString::fromUtf8("BASIC V1"));
	actionBootMode[1]->setText(QString::fromUtf8("BASIC V2"));	
	actionBootMode[2]->setText(QString::fromUtf8("Playbox BASIC"));
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



