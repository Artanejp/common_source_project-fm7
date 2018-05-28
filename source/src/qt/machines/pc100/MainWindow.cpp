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
#include "emu.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "qt_main.h"

extern config_t config;

Action_Control_PC100::Action_Control_PC100(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	mz_binds = new Object_Menu_Control_PC100(parent, p);
}

Action_Control_PC100::~Action_Control_PC100(){
	delete mz_binds;
}

Object_Menu_Control_PC100::Object_Menu_Control_PC100(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_PC100::~Object_Menu_Control_PC100(){
}

void Object_Menu_Control_PC100::do_set_monitor_type(void)
{
#if defined(USE_MONITOR_TYPE)
	int n = getValue1();
	if(n < 0) return;
	if(n >= USE_MONITOR_TYPE) return;
	config.monitor_type = n;
	emit sig_update_config();
#endif
}

void META_MainWindow::do_pc100_update_config(void)
{
	emit sig_emu_update_config();
}

void META_MainWindow::setupUI_Emu(void)
{
	int i;
	QString tmps;
	menuMachine->setVisible(true);
#if defined(USE_MONITOR_TYPE)
	menuDisplayType = new QMenu(this);
	actionGroup_DisplayType = new QActionGroup(this);
	actionGroup_DisplayType->setExclusive(true);
	for(int i = 0; i < USE_MONITOR_TYPE; i++) {
		action_DisplayType[i] = new Action_Control_PC100(this, using_flags);
		action_DisplayType[i]->setCheckable(true);
		action_DisplayType[i]->setVisible(true);
		action_DisplayType[i]->pc100_binds->setValue1(i);
		actionGroup_DisplayType->addAction(action_DisplayType[i]);
		if(config.monitor_type == i) action_DisplayType[i]->setChecked(true);
		connect(action_DisplayType[i], SIGNAL(triggered()), action_DisplayType[i]->pc100_binds, SLOT(do_set_monitor_type()));
		connect(action_DisplayType[i]->pc100_binds, SIGNAL(sig_update_config()), this, SLOT(do_pc100_update_config()));
		menuDisplayType->addAction(action_DisplayType[i]);
	}
	menuMachine->addAction(menuDisplayType->menuAction());
#endif
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
	action_DisplayType[0]->setText(QApplication::translate("Machine", "Horizonal", 0));
	action_DisplayType[0]->setToolTip(QApplication::translate("Machine", "Use DISPLAY as horizonal.", 0));
	action_DisplayType[1]->setText(QApplication::translate("Machine", "Vertical", 0));
	action_DisplayType[1]->setToolTip(QApplication::translate("Machine", "Use DISPLAY as vertical.", 0));
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



