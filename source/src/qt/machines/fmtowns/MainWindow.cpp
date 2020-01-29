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

//QT_BEGIN_NAMESPACE


void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("", false);
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
	actionJoystickType[0]->setText(QApplication::translate("Machine", "2 buttons", 0));
	actionJoystickType[0]->setToolTip(QApplication::translate("Machine", "Connect 2 buttons Towns PAD to PORTs.", 0));
	actionJoystickType[1]->setText(QApplication::translate("Machine", "6 buttons", 0));
	actionJoystickType[1]->setToolTip(QApplication::translate("Machine", "Connect 6 buttons Towns PAD to PORTs.", 0));
	menuJoystickType->setTitle(QApplication::translate("Machine", "Towns PAD", 0));
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



