/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QVariant>
#include <QtGui>
#include "vm.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE

extern config_t config;

Action_Control_MZ700::Action_Control_MZ700(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	mz_binds = new Object_Menu_Control_MZ700(parent, p);
}

Action_Control_MZ700::~Action_Control_MZ700(){
	delete mz_binds;
}

Object_Menu_Control_MZ700::Object_Menu_Control_MZ700(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_MZ700::~Object_Menu_Control_MZ700(){
}

void Object_Menu_Control_MZ700::do_monitor_type(void)
{
	emit sig_monitor_type(getValue1());
}

void META_MainWindow::set_monitor_type(int num)
{
#ifdef USE_MONITOR_TYPE
	if((num < 0) || (num >= USE_MONITOR_TYPE)) return;
	config.monitor_type = num;
	this->do_emu_update_config();
#endif
}

void META_MainWindow::do_set_pcg(bool flag)
{
#ifdef _MZ700
	this->set_dipsw(0, flag);
	//this->do_emu_update_config();
#endif
}

void META_MainWindow::setupUI_Emu(void)
{
#if !defined(_MZ800)
	//menuMachine->setVisible(false);
#endif   
#if defined(_MZ700)
	action_PCG700 = new QAction(menuMachine);
	action_PCG700->setCheckable(true);
	if((config.dipswitch & 0x0001) != 0) action_PCG700->setChecked(true);
	connect(action_PCG700, SIGNAL(toggled(bool)), this, SLOT(do_set_pcg(bool)));
	menuMachine->addAction(action_PCG700);
	menuMachine->addSeparator();
#endif
#if defined(USE_BOOT_MODE)
	menuBootMode = new QMenu(menuMachine);
	menuBootMode->setObjectName(QString::fromUtf8("menuControl_BootMode"));
	menuMachine->addAction(menuBootMode->menuAction());
	ConfigCPUBootMode(USE_BOOT_MODE);
#endif
#if defined(USE_MONITOR_TYPE)
	{
		int ii;
		menuMonitorType = new QMenu(menuMachine);
		menuMonitorType->setObjectName(QString::fromUtf8("menuControl_MonitorType"));
		menuMachine->addAction(menuMonitorType->menuAction());
		
		actionGroup_MonitorType = new QActionGroup(this);
		actionGroup_MonitorType->setExclusive(true);
		for(ii = 0; ii < USE_MONITOR_TYPE; ii++) {
			actionMonitorType[ii] = new Action_Control_MZ700(this, using_flags);
			actionGroup_MonitorType->addAction(actionMonitorType[ii]);
			actionMonitorType[ii]->setCheckable(true);
			actionMonitorType[ii]->setVisible(true);
			actionMonitorType[ii]->mz_binds->setValue1(ii);
			if(config.monitor_type == ii) actionMonitorType[ii]->setChecked(true);
			menuMonitorType->addAction(actionMonitorType[ii]);
			connect(actionMonitorType[ii], SIGNAL(triggered()),
					actionMonitorType[ii]->mz_binds, SLOT(do_monitor_type()));
			connect(actionMonitorType[ii]->mz_binds, SIGNAL(sig_monitor_type(int)),
					this, SLOT(set_monitor_type(int)));
		}
	}
#endif

}

void META_MainWindow::retranslateUi(void)
{
	retranslateControlMenu(" ",  true);
	retranslateFloppyMenu(0, 0);
	retranslateFloppyMenu(1, 1);
	retranslateQuickDiskMenu(0, 0);
	retranslateCMTMenu();
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
	
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
#if defined(_MZ800)
	menuBootMode->setTitle(QApplication::translate("Machine", "BOOT Mode", 0));
	actionBootMode[0]->setText(QString::fromUtf8("MZ-800"));
	actionBootMode[1]->setText(QString::fromUtf8("MZ-700"));
   
	menuMonitorType->setTitle("Monitor Type");
	menuMonitorType->setToolTipsVisible(true);
	actionMonitorType[0]->setText(QApplication::translate("MainWindow", "Color", 0));
	actionMonitorType[1]->setText(QApplication::translate("MainWindow", "Monochrome", 0));
	actionMonitorType[0]->setToolTip(QApplication::translate("MainWindow", "Use color CRT.", 0));
	actionMonitorType[1]->setToolTip(QApplication::translate("MainWindow", "Use monochrome CRT.", 0));
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));;
#elif defined(_MZ700)
	action_PCG700->setText(QApplication::translate("MainWindow", "PCG-700", 0));
	action_PCG700->setToolTip(QApplication::translate("MainWindow", "HAL laboratory PCG-700 PCG.", 0));
#endif
#if defined(_MZ1500)
	actionPrintDevice[1]->setText(QString::fromUtf8("MZ-1P17"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MainWindow", "Sharp MZ-1P17 kanji thermal printer.", 0));
#endif
#ifdef USE_DEBUGGER
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



