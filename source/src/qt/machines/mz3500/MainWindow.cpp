/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QVariant>
#include <QApplication>
#include <QtGui>
#include <QActionGroup>
#include <QMenu>

#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE

extern config_t config;

Action_Control_MZ3500::Action_Control_MZ3500(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	mz_binds = new Object_Menu_Control_MZ3500(parent, p);
}

Action_Control_MZ3500::~Action_Control_MZ3500(){
	delete mz_binds;
}

Object_Menu_Control_MZ3500::Object_Menu_Control_MZ3500(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_MZ3500::~Object_Menu_Control_MZ3500(){
}

void Object_Menu_Control_MZ3500::set_dipsw(bool flag)
{
	emit sig_dipsw(getValue1(), flag);
}
	
void Object_Menu_Control_MZ3500::do_set_monitor_type(void)
{
#if defined(USE_MONITOR_TYPE)
	int n = getValue1();
	if(n < 0) return;
	if(n >= USE_MONITOR_TYPE) return;
	config.monitor_type = n;
	emit sig_update_config();
#endif
}

void META_MainWindow::do_mz35_update_config(void)
{
	emit sig_emu_update_config();
}

void META_MainWindow::setupUI_Emu(void)
{
	int i;
	QString tmps;
	menuMachine->setVisible(true);
	menu_Emu_DipSw = new QMenu(menuMachine);
	menu_Emu_DipSw->setObjectName(QString::fromUtf8("menu_DipSw"));
	actionGroup_DipSw = new QActionGroup(this);
	actionGroup_DipSw->setExclusive(false);

	menuMachine->addAction(menu_Emu_DipSw->menuAction());
	
	for(i = 0; i < 3; i++) {
      	action_Emu_DipSw[i] = new Action_Control_MZ3500(this, using_flags);
        action_Emu_DipSw[i]->setCheckable(true);
        tmps.number(i + 1);
        tmps = QString::fromUtf8("actionEmu_DipSw") + tmps;
        action_Emu_DipSw[i]->setObjectName(tmps);
		menu_Emu_DipSw->addAction(action_Emu_DipSw[i]);
			
		actionGroup_DipSw->addAction(action_Emu_DipSw[i]);
		connect(action_Emu_DipSw[i], SIGNAL(toggled(bool)),
				action_Emu_DipSw[i]->mz_binds, SLOT(set_dipsw(bool)));
		connect(action_Emu_DipSw[i]->mz_binds, SIGNAL(sig_dipsw(int, bool)),
				this, SLOT(set_dipsw(int, bool)));
	
	}
	action_Emu_DipSw[0]->mz_binds->setValue1(3);
	action_Emu_DipSw[1]->mz_binds->setValue1(7);
	action_Emu_DipSw[2]->mz_binds->setValue1(8);
	if(((1 << 3) & config.dipswitch) != 0) action_Emu_DipSw[0]->setChecked(true);
	if(((1 << 7) & config.dipswitch) != 0) action_Emu_DipSw[1]->setChecked(true);
	if(((1 << 8) & config.dipswitch) != 0) action_Emu_DipSw[2]->setChecked(true);
	
#if defined(USE_MONITOR_TYPE)
	menuDisplayType = new QMenu(this);
	actionGroup_DisplayType = new QActionGroup(this);
	actionGroup_DisplayType->setExclusive(true);
	for(int i = 0; i < USE_MONITOR_TYPE; i++) {
		action_DisplayType[i] = new Action_Control_MZ3500(this, using_flags);
		action_DisplayType[i]->setCheckable(true);
		action_DisplayType[i]->setVisible(true);
		action_DisplayType[i]->mz_binds->setValue1(i);
		actionGroup_DisplayType->addAction(action_DisplayType[i]);
		if(config.monitor_type == i) action_DisplayType[i]->setChecked(true);
		connect(action_DisplayType[i], SIGNAL(triggered()), action_DisplayType[i]->mz_binds, SLOT(do_set_monitor_type()));
		connect(action_DisplayType[i]->mz_binds, SIGNAL(sig_update_config()), this, SLOT(do_mz35_update_config()));
		menuDisplayType->addAction(action_DisplayType[i]);
	}
	menuMachine->addAction(menuDisplayType->menuAction());
#endif
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("Halt",  true);
	actionReset->setToolTip(QApplication::translate("MainWindow", "Do system reset.", 0));
	actionSpecial_Reset->setToolTip(QApplication::translate("MainWindow", "HALT a machine.", 0));
   
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
	menu_Emu_DipSw->setTitle(QApplication::translate("MainWindow", "Dip Switches", 0));
	action_Emu_DipSw[0]->setText(QApplication::translate("MainWindow", "SW4: Period for Decimal Point", 0));
	action_Emu_DipSw[1]->setText(QApplication::translate("MainWindow", "FD1: Normally Capital Letter", 0));
	action_Emu_DipSw[2]->setText(QApplication::translate("MainWindow", "P/M: 3500 CG for 200 Line CRT", 0));

	actionPrintDevice[1]->setText(QApplication::translate("MainWindow", "MZ-1P17", 0));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MainWindow", "Sharp MZ-1P17 kanji thermal printer.", 0));
#if defined(USE_MONITOR_TYPE)
	menuDisplayType->setTitle(QApplication::translate("MachineMZ2500", "Monitor Type:", 0));
	action_DisplayType[0]->setText(QApplication::translate("MachineMZ2500", "400Lines, Analog.", 0));
	action_DisplayType[1]->setText(QApplication::translate("MachineMZ2500", "400Lines, Digital.", 0));
	action_DisplayType[2]->setText(QApplication::translate("MachineMZ2500", "200Lines, Analog.", 0));
	action_DisplayType[3]->setText(QApplication::translate("MachineMZ2500", "200Lines, Digital.", 0));
#endif
#if defined(USE_DEBUGGER)
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
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



