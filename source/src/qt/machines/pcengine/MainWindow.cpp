/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for PC Engine .
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
#include "menu_compactdisc.h"
#include "menu_cart.h"

//QT_BEGIN_NAMESPACE
	

void META_MainWindow::setupUI_Emu(void)
{
}

void META_MainWindow::retranslateUi(void)
{
	retranslateControlMenu("",  false);
	retranslateCartMenu(0, 1);
	retranslateCDROMMenu();
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();

	menuDeviceType->setTitle(QApplication::translate("MainWindow", "Joy PAD Type", 0));
	actionDeviceType[0]->setText(QApplication::translate("MainWindow", "2-Buttons Joy Pad", 0));
	actionDeviceType[1]->setText(QApplication::translate("MainWindow", "6-Buttons Joy Pad", 0));
	actionDeviceType[2]->setText(QApplication::translate("MainWindow", "2-Buttons with Multi-Tap", 0));
	actionDeviceType[3]->setText(QApplication::translate("MainWindow", "6-Buttons with Multi-Tap", 0));

	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
	actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));

	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));

	menu_Cart[0]->setTitle(QApplication::translate("MainWindow", "HuCARD", 0));
	menu_CDROM->setTitle(QApplication::translate("MainWindow", "CD-ROM^2", 0));
	
	menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0));
	actionHelp_AboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0));
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



