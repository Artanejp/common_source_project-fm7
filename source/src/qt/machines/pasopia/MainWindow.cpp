/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for PASOPIA/PASOPIA7 .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QtGui>
#include <QMenu>

#include "menuclasses.h"
#include "qt_main.h"
#include "menu_binary.h"

void META_MainWindow::setupUI_Emu(void)
{
#if defined(_PASOPIA)
	ConfigCPUBootMode(5);
#endif
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(" ",  false);
	retranslateOpMenuZ80(true);

#if defined(_PASOPIA)
	retranslateBinaryMenu(0, 1);
	menu_BINs[0]->setTitle(QApplication::translate("MainWindow", "RAMPAC", 0));
	menuBootMode->setTitle(QApplication::translate("Machine", "BOOT Mode", 0));
	menuBootMode->setToolTipsVisible(true);
	actionBootMode[0]->setText(QString::fromUtf8("T-BASIC Ver 1.0"));
	actionBootMode[1]->setText(QString::fromUtf8("T-BASIC Ver 1.1"));
	actionBootMode[2]->setText(QString::fromUtf8("OA-BASIC (Disk)"));
	actionBootMode[3]->setText(QString::fromUtf8("OA-BASIC"));
	actionBootMode[4]->setText(QString::fromUtf8("Mini Pascal"));

	menuDeviceType->setTitle(QApplication::translate("MainWindow", "Extra connector", 0));
	menuDeviceType->setToolTipsVisible(true);
	actionDeviceType[0]->setText(QApplication::translate("MainWindow", "RAMPAC", 0));
	actionDeviceType[1]->setText(QApplication::translate("MainWindow", "Kanji ROM", 0));
	actionDeviceType[2]->setText(QApplication::translate("MainWindow", "Joystick", 0));

#elif defined(_PASOPIA7)
	retranslateBinaryMenu(0, 1);
	menu_BINs[0]->setTitle(QApplication::translate("MainWindow", "RAMPAC1", 0));
	//actionSave_BIN[0]->setVisible(false);
	retranslateBinaryMenu(1, 2);
	menu_BINs[1]->setTitle(QApplication::translate("MainWindow", "RAMPAC2", 0));
	//actionSave_BIN[1]->setVisible(false);
	menuDeviceType->setToolTipsVisible(true);
#endif
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));

#if defined(USE_DEBUGGER)
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
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
