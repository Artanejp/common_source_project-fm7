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

//QT_BEGIN_NAMESPACE

Action_Control_MZ3500::Action_Control_MZ3500(QObject *parent) : Action_Control(parent)
{
	mz_binds = new Object_Menu_Control_MZ3500(parent);
}

Action_Control_MZ3500::~Action_Control_MZ3500(){
	delete mz_binds;
}

Object_Menu_Control_MZ3500::Object_Menu_Control_MZ3500(QObject *parent) : Object_Menu_Control(parent)
{
}

Object_Menu_Control_MZ3500::~Object_Menu_Control_MZ3500(){
}

void Object_Menu_Control_MZ3500::set_dipsw(bool flag)
{
	emit sig_dipsw(getValue1(), flag);
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
      	action_Emu_DipSw[i] = new Action_Control_MZ3500(this);
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
	
}

void META_MainWindow::retranslateUi(void)
{
	int i;
	retranslateControlMenu("Halt",  true);
	retranslateFloppyMenu(0, 1);
	retranslateFloppyMenu(1, 2);
	retranslateFloppyMenu(2, 3);
	retranslateFloppyMenu(3, 4);
	retranslateMachineMenu();

	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
   
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
	menu_Emu_DipSw->setTitle(QApplication::translate("MainWindow", "Dip Switches", 0));
	action_Emu_DipSw[0]->setText(QApplication::translate("MainWindow", "SW4: Period for Decimal Point", 0));
	action_Emu_DipSw[1]->setText(QApplication::translate("MainWindow", "FD1: Normally Capital Letter", 0));
	action_Emu_DipSw[2]->setText(QApplication::translate("MainWindow", "P/M: 3500 CG for 200 Line CRT", 0));

//	actionPrintDevice[1]->setText(QApplication::translate("MainWindow", "Write to file", 0));
	actionPrintDevice[1]->setText(QApplication::translate("MainWindow", "MZ-1P17", 0));

		// Set Labels
} // retranslateUi



META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



