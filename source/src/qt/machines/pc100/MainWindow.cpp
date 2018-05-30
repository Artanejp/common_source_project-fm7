/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for PC-100 .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QMenu>
#include "emu.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "qt_main.h"

extern config_t config;

Action_Control_PC100::Action_Control_PC100(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	pc100_binds = new Object_Menu_Control_PC100(parent, p);
}

Action_Control_PC100::~Action_Control_PC100(){
	delete pc100_binds;
}

Object_Menu_Control_PC100::Object_Menu_Control_PC100(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_PC100::~Object_Menu_Control_PC100(){
}


void META_MainWindow::setupUI_Emu(void)
{
	int i;
	QString tmps;
	menuMachine->setVisible(true);
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("",  false);
#if defined(USE_DRIVE_TYPE)
	menuDriveType->setTitle(QApplication::translate("MachinePC100", "Floppy Type", 0));
	actionDriveType[0]->setText(QApplication::translate("MachinePC100", "2D", 0));
	actionDriveType[1]->setText(QApplication::translate("MachinePC100", "2DD", 0));
#endif
#if defined(USE_MONITOR_TYPE)
	actionMonitorType[0]->setText(QApplication::translate("Machine", "Horizonal", 0));
	actionMonitorType[0]->setToolTip(QApplication::translate("Machine", "Use DISPLAY as horizonal.", 0));
	actionMonitorType[1]->setText(QApplication::translate("Machine", "Vertical", 0));
	actionMonitorType[1]->setToolTip(QApplication::translate("Machine", "Use DISPLAY as vertical.", 0));
#endif	
#if defined(USE_DEBUGGER)
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
	// Set Labels
} // retranslateUi



META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



