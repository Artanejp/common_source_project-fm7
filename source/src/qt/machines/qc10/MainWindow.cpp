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
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"


Action_Control_QC10::Action_Control_QC10(QObject *parent) : Action_Control(parent)
{
	qc_binds = new Object_Menu_Control_QC10(parent);
}

Action_Control_QC10::~Action_Control_QC10(){
	delete qc_binds;
}

Object_Menu_Control_QC10::Object_Menu_Control_QC10(QObject *parent) : Object_Menu_Control(parent)
{
}

Object_Menu_Control_QC10::~Object_Menu_Control_QC10(){
}

void Object_Menu_Control_QC10::set_dipsw(bool flag)
{
	emit sig_dipsw(getValue1(), flag);
}
	

void META_MainWindow::retranslateUi(void)
{
	retranslateControlMenu(" ", false);
	retranslateFloppyMenu(0, 1);
	retranslateFloppyMenu(1, 2);
#if defined(USE_FD3)	
	retranslateFloppyMenu(2, 3);
#endif
#if defined(USE_FD4)	
	retranslateFloppyMenu(3, 4);
#endif
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
	QString tmps;
	QString n_tmps;
	int i;
	menu_Emu_DipSw->setTitle(QApplication::translate("MainWindow", "DIP Switches", 0));
	for(i = 0; i < 8; i++) {
		tmps = QApplication::translate("MainWindow", "DIP Switch #", 0);
		n_tmps.setNum(i + 1);
		action_Emu_DipSw[i]->setText(tmps + n_tmps);
	}
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
} // retranslateUi

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
	
	for(i = 0; i < 8; i++) {
		action_Emu_DipSw[i] = new Action_Control_QC10(this);
		action_Emu_DipSw[i]->setCheckable(true);
		action_Emu_DipSw[i]->qc_binds->setValue1(i);
		tmps.number(i + 1);
		tmps = QString::fromUtf8("actionEmu_DipSw") + tmps;
		action_Emu_DipSw[i]->setObjectName(tmps);
		menu_Emu_DipSw->addAction(action_Emu_DipSw[i]);
		if((config.dipswitch & (1 << i)) != 0) action_Emu_DipSw[i]->setChecked(true);
		
		actionGroup_DipSw->addAction(action_Emu_DipSw[i]);
		connect(action_Emu_DipSw[i], SIGNAL(toggled(bool)),
				action_Emu_DipSw[i]->qc_binds, SLOT(set_dipsw(bool)));
		connect(action_Emu_DipSw[i]->qc_binds, SIGNAL(sig_dipsw(int, bool)),
				this, SLOT(set_dipsw(int, bool)));
	}

}


META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}


