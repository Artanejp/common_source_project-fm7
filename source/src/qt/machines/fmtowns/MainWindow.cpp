/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for Babbage2nd .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */
#include <QApplication>
#include <QtCore/QVariant>
#include <QtGui>
#include "menuclasses.h"
#include "commonclasses.h"

#include "emu.h"
#include "qt_main.h"
#include "../../vm/fmtowns/fmtowns.h"
#include "menu_binary.h"
#include "menu_cart.h"

//QT_BEGIN_NAMESPACE


void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("Reset with CD", true);
	actionSpecial_Reset[0]->setText(QApplication::translate("Machine", "Reset with CD", 0));
	actionSpecial_Reset[0]->setToolTip(QApplication::translate("Machine", "Reset and boot from CD-ROM", 0));
	for(int i = 1; i < 5; i++) {
		QString tmps2, tmps3;
		QString numname = QString::number(i - 1);
		tmps2 = QApplication::translate("Machine", "Reset with F%1", 0).arg(numname);
		tmps3 = QApplication::translate("Machine", "Reset and boot from FLOPPY #%1", 0).arg(numname);
		actionSpecial_Reset[i]->setText(tmps2);
		actionSpecial_Reset[i]->setToolTip(tmps3);
	}
	for(int i = 5; i < 10; i++) {
		QString tmps2, tmps3;
		QString numname = QString::number(i - 5);
		tmps2 = QApplication::translate("Machine", "Reset with H%1", 0).arg(numname);
		tmps3 = QApplication::translate("Machine", "Reset and boot from HDD #%1", 0).arg(numname);
		actionSpecial_Reset[i]->setText(tmps2);
		actionSpecial_Reset[i]->setToolTip(tmps3);
	}
	actionSpecial_Reset[10]->setText(QApplication::translate("Machine", "Reset with ICM", 0));
	actionSpecial_Reset[10]->setToolTip(QApplication::translate("Machine", "Reset with boot from IC CARD #0", 0));
	actionSpecial_Reset[11]->setText(QApplication::translate("Machine", "Reset with DEBUG", 0));
	actionSpecial_Reset[11]->setToolTip(QApplication::translate("Machine", "Reset with DEBUGGING MODE", 0));
# if defined(USE_MOUSE_TYPE)
	menuMouseType->setTitle(QApplication::translate("Machine", "Mouse", 0));
	menuMouseType->setToolTipsVisible(true);
	actionMouseType[0]->setText(QApplication::translate("Machine", "none", 0));
	actionMouseType[0]->setToolTip(QApplication::translate("Machine", "Not connect mouse.", 0));
	actionMouseType[1]->setText(QApplication::translate("Machine", "PAD port1", 0));
	actionMouseType[1]->setToolTip(QApplication::translate("Machine", "Connect mouse to PAD port #1.", 0));
	actionMouseType[2]->setText(QApplication::translate("Machine", "PAD port2", 0));
	actionMouseType[2]->setToolTip(QApplication::translate("Machine", "Connect mouse to PAD port #2.", 0));
# endif	
#if defined(USE_JOYSTICK_TYPE)
	actionJoystickType[0]->setText(QApplication::translate("Machine", "None", 0));
	actionJoystickType[0]->setToolTip(QApplication::translate("Machine", "NotConnected.", 0));
	actionJoystickType[1]->setText(QApplication::translate("Machine", "2 buttons", 0));
	actionJoystickType[1]->setToolTip(QApplication::translate("Machine", "Connect 2 buttons Towns PAD to PORTs.", 0));
	actionJoystickType[2]->setText(QApplication::translate("Machine", "6 buttons", 0));
	actionJoystickType[2]->setToolTip(QApplication::translate("Machine", "Connect 6 buttons Towns PAD to PORTs.", 0));
	menuJoystickType->setTitle(QApplication::translate("Machine", "Towns PAD", 0));
#endif
#if defined(USE_CART)
	if(menu_Cart[0] != NULL) {
		menu_Cart[0]->setTitle(QApplication::translate("MainWindow", "IC1", 0));
	}
	if(menu_Cart[1] != NULL) {
		menu_Cart[1]->setTitle(QApplication::translate("MainWindow", "IC2", 0));
	}
#endif
//	menu_BINs[0]->setTitle(QApplication::translate("MenuBABBAGE", "RAM", 0));
	//menuMachine->setVisible(false);
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



