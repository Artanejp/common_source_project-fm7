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

#include "config.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

extern config_t config;
//QT_BEGIN_NAMESPACE
Object_Menu_Control_TK::Object_Menu_Control_TK(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_TK::~Object_Menu_Control_TK()
{
}


void Object_Menu_Control_TK::do_set_dipsw(bool flag)
{
	int bitpos = getValue1();
	uint32_t bit = 0x00000001;
	if((bitpos < 0) || (bitpos >= 32)) return;
	bit = bit << bitpos;
	if(flag) {
		config.dipswitch = config.dipswitch | bit;
	} else {
		config.dipswitch = config.dipswitch & ((uint32_t)~bit);
	}
}

Action_Control_TK::Action_Control_TK(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	tk_binds = new Object_Menu_Control_TK(parent, p);
	tk_binds->setValue1(0);
}

Action_Control_TK::~Action_Control_TK()
{
	delete tk_binds;
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("", false);
	// Will implement : RAM LOAD/SAVE

	if(menu_BINs[0] != NULL) menu_BINs[0]->setTitle(QApplication::translate("MachineTK80BS", "RAM", 0));
#if defined(_TK80BS)	
	if(menu_CMT[1] != NULL) menu_CMT[1]->setTitle(QApplication::translate("MachineTK80BS", "BS-CMT", 0));
#endif
	action_DipSWs[0]->setText(QApplication::translate("MachineTK80BS", "STEP/AUTO", 0));
	action_DipSWs[0]->setToolTip(QApplication::translate("MachineTK80BS", "If enabled, interrupt per instruction.\nUseful for debugging.", 0));
	action_DipSWs[0]->setToolTipVisible(true);
	
	menuBootMode->setTitle(QApplication::translate("MachineTK80BS", "BOOT Mode", 0));
	menuBootMode->setToolTipsVisible(true);
#if defined(_TK80BS)
	actionBootMode[0]->setText(QString::fromUtf8("L1 BASIC"));
	actionBootMode[1]->setText(QString::fromUtf8("L2 BASIC"));	
	actionBootMode[0]->setToolTip(QApplication::translate("MachineTK80BS", "Use L1 BASIC.", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("MachineTK80BS", "Use L2 BASIC.", 0));
#endif
	
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
	
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
	ConfigCPUBootMode(2);
	for(int i = 0; i < 4; i++) {
		action_DipSWs[i] = NULL;
	}
	menuMachine->addSeparator();
	uint32_t _bit = 0x00000001;
	for(int i = 0; i < 1; i++) {
		action_DipSWs[i] = Action_Control_TK(this, using_flags);
		action_DipSWs[i]->setCheckable(true);
		action_DipSWs[i]->setVisible(true);
		action_DipSWs[i]->setEnabled(true);
		menuMachine->addAction(action_DipSWs[i]);
		if((config.dipswitch & _bit) != 0) action_DipSWs[i]->setChecked(true);
		action_DipSWs[i]->setValue1(i);
		connect(action_DipSWs[i], SIGNAL(toggled(bool)), action_DipSWs[i]->binds, SLOT(do_set_dipsw(bool)));
		connect(action_DipSWs[i]->binds, SIGNAL(sig_emu_update_config()), this, SLOT(do_emu_update_config()));
		bit <<= 1;
	}
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



