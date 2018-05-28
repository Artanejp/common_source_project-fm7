/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QMenu>
#include <QActionGroup>

#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

extern config_t config;

Action_Control_MZ80::Action_Control_MZ80(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	mz_binds = new Object_Menu_Control_MZ80(parent, p);
}

Action_Control_MZ80::~Action_Control_MZ80(){
	delete mz_binds;
}

Object_Menu_Control_MZ80::Object_Menu_Control_MZ80(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_MZ80::~Object_Menu_Control_MZ80(){
}

void Object_Menu_Control_MZ80::set_dipsw(bool flag)
{
	emit sig_dipsw(getValue1(), flag);
}

void Object_Menu_Control_MZ80::do_set_monitor_type(void)
{
#if defined(USE_MONITOR_TYPE)
	int n = getValue1();
	if(n < 0) return;
	if(n >= USE_MONITOR_TYPE) return;
	config.monitor_type = n;
	emit sig_update_config();
#endif
}

void META_MainWindow::do_mz_update_config(void)
{
	emit sig_emu_update_config();
}

void META_MainWindow::setupUI_Emu(void)
{
	QString tmps;
	menuMachine->setVisible(true);
	action_Emu_DipSw = new Action_Control_MZ80(this, using_flags);
	action_Emu_DipSw->setCheckable(true);
	tmps = QString::fromUtf8("actionEmu_DipSw0");
	action_Emu_DipSw->setObjectName(tmps);
	action_Emu_DipSw->mz_binds->setValue1(0);
	menuMachine->addAction(action_Emu_DipSw);
	
	if((1 & config.dipswitch) != 0) action_Emu_DipSw->setChecked(true);

	connect(action_Emu_DipSw, SIGNAL(toggled(bool)),
			action_Emu_DipSw->mz_binds, SLOT(set_dipsw(bool)));
	connect(action_Emu_DipSw->mz_binds, SIGNAL(sig_dipsw(int, bool)),
			this, SLOT(set_dipsw(int, bool)));

#if defined(USE_MONITOR_TYPE)
	menuDisplayType = new QMenu(this);
	actionGroup_DisplayType = new QActionGroup(this);
	actionGroup_DisplayType->setExclusive(true);
	for(int i = 0; i < USE_MONITOR_TYPE; i++) {
		action_DisplayType[i] = new Action_Control_MZ80(this, using_flags);
		action_DisplayType[i]->setCheckable(true);
		action_DisplayType[i]->setVisible(true);
		action_DisplayType[i]->mz_binds->setValue1(i);
		actionGroup_DisplayType->addAction(action_DisplayType[i]);
		if(config.monitor_type == i) action_DisplayType[i]->setChecked(true);
		connect(action_DisplayType[i], SIGNAL(triggered()), action_DisplayType[i]->mz_binds, SLOT(do_set_monitor_type()));
		connect(action_DisplayType[i]->mz_binds, SIGNAL(sig_update_config()), this, SLOT(do_mz_update_config()));
		menuDisplayType->addAction(action_DisplayType[i]);
	}
	menuMachine->addAction(menuDisplayType->menuAction());
#endif
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(" ",  false);
#if defined(_MZ80A) || defined(_MZ80K)	
	action_Emu_DipSw->setText(QApplication::translate("MachineMZ80K", "PCG-8000", 0));
	action_Emu_DipSw->setToolTip(QApplication::translate("MachineMZ80K", "HAL laboratory PCG-8000 PCG.", 0));
#elif defined(_MZ1200)
	action_Emu_DipSw->setText(QApplication::translate("MachineMZ80K", "PCG-1200", 0));
	action_Emu_DipSw->setToolTip(QApplication::translate("MachineMZ80K", "HAL laboratory PCG-1200 PCG.", 0));
#endif

#if defined(_MZ1200) || defined(_MZ80K) || defined(_MZ80C)
	actionPrintDevice[1]->setText(QString::fromUtf8("MZ-80P3"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineMZ80K", "Sharp MZ-80P3 thermal printer.", 0));
#elif defined(_MZ80A)
	actionPrintDevice[1]->setText(QString::fromUtf8("MZ-80P4"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineMZ80K", "Sharp MZ-80P4 thermal printer.", 0));
//#else
//	actionPrintDevice[1]->setText(QString::fromUtf8("MZ-1P17"));
//	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineMZ80K", "Sharp MZ-1P17 kanji thermal printer.", 0));
#endif
	actionPrintDevice[2]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MachineMZ80K", "NEC PC-PR201 printer.", 0));
	actionPrintDevice[2]->setEnabled(false);
	
	this->setWindowTitle(QApplication::translate("MachineMZ80K", "MachineMZ80K", 0));
#if defined(USE_DRIVE_TYPE)
	menuDriveType->setTitle(QApplication::translate("MachineMZ80K", "Floppy Type", 0));
	actionDriveType[0]->setText(QApplication::translate("MachineMZ80K", "2D", 0));
	actionDriveType[1]->setText(QApplication::translate("MachineMZ80K", "2DD", 0));
#endif
#if defined(USE_MONITOR_TYPE)
	menuDisplayType->setTitle(QApplication::translate("MachineMZ80K", "Monitor Type", 0));
	action_DisplayType[0]->setText(QApplication::translate("MachineMZ80K", "WHITE (MZ-80K)", 0));
	action_DisplayType[0]->setToolTip(QApplication::translate("MachineMZ80K", "Using MZ-80K's B&W DISPLAY.", 0));
	action_DisplayType[1]->setText(QApplication::translate("MachineMZ80K", "GREEN (MZ-80C)", 0));
	action_DisplayType[1]->setToolTip(QApplication::translate("MachineMZ80K", "Using MZ-80C's GREEN DISPLAY.", 0));
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



