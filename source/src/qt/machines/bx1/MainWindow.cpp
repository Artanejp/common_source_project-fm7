/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for Super Casette Vision .
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

//QT_BEGIN_NAMESPACE
	
Object_Menu_Control_BX1::Object_Menu_Control_BX1(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_BX1::~Object_Menu_Control_BX1()
{
}

void Object_Menu_Control_BX1::do_set_dipsw(bool flag)
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

Action_Control_BX1::Action_Control_BX1(QObject *parent, USING_FLAGS *p, int num) : Action_Control(parent, p)
{
	bx1_binds = new Object_Menu_Control_BX1(parent, p);
	bx1_binds->setValue1(num);
}

Action_Control_BX1::~Action_Control_BX1()
{
	delete bx1_binds;
}

void META_MainWindow::setupUI_Emu(void)
{
	menuMachine->addSeparator();
	uint32_t _bit = 0x00000001;
	for(int i = 0; i < 4; i++) {
		action_DipSWs[i] = new Action_Control_BX1(this, using_flags, i);
		action_DipSWs[i]->setCheckable(true);
		action_DipSWs[i]->setVisible(true);
		action_DipSWs[i]->setEnabled(true);
		menuMachine->addAction(action_DipSWs[i]);
		if((config.dipswitch & _bit) != 0) action_DipSWs[i]->setChecked(true);

		connect(action_DipSWs[i], SIGNAL(toggled(bool)), action_DipSWs[i]->binds, SLOT(do_set_dipsw(bool)));
		connect(action_DipSWs[i]->binds, SIGNAL(sig_emu_update_config()), this, SLOT(do_emu_update_config()));
		_bit <<= 1;
	}
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("",  false);
	action_DipSWs[0]->setText(QApplication::translate("MachineBX1", "AUTO PRINT Switch", 0));
	action_DipSWs[1]->setText(QApplication::translate("MachineBX1", "PROG.SELECT Switch", 0));
	action_DipSWs[2]->setText(QApplication::translate("MachineBX1", "JPN/ENG Jumper", 0));
	action_DipSWs[0]->setVisible(true);
	action_DipSWs[1]->setVisible(true);
	action_DipSWs[2]->setVisible(true);
	action_DipSWs[3]->setVisible(false);

	actionPrintDevice[3]->setText(QApplication::translate("MachineBX1", "None", 0));
	
	actionPrintDevice[0]->setVisible(true);
	actionPrintDevice[1]->setVisible(false);
	actionPrintDevice[2]->setVisible(false);
	actionPrintDevice[3]->setVisible(true);


	actionPrintDevice[0]->setEnabled(true);
	actionPrintDevice[1]->setEnabled(false);
	actionPrintDevice[1]->setEnabled(false);
	actionPrintDevice[3]->setEnabled(true);

#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
} // retranslateUi



META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	for(int i = 0; i < 4; i++) {
		action_DipSWs[i] = NULL;
	}
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



