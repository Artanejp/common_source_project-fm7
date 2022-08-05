/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for PC Engine .
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
#include "menu_compactdisc.h"
#include "menu_cart.h"

//QT_BEGIN_NAMESPACE
	

void META_MainWindow::setupUI_Emu(void)
{
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("",  false);

	menuJoystickType->setTitle(QApplication::translate("MainWindow", "Joy PAD Type", 0));
	actionJoystickType[0]->setText(QApplication::translate("MainWindow", "2-Buttons Joy Pad", 0));
	actionJoystickType[1]->setText(QApplication::translate("MainWindow", "6-Buttons Joy Pad", 0));
	actionJoystickType[2]->setText(QApplication::translate("MainWindow", "2-Buttons with Multi-Tap", 0));
	actionJoystickType[3]->setText(QApplication::translate("MainWindow", "6-Buttons with Multi-Tap", 0));


	menu_Cart[0]->setTitle(QApplication::translate("MainWindow", "HuCARD", 0));
	menu_CDROM[0]->setTitle(QApplication::translate("MainWindow", "CD-ROM^2", 0));
	
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



