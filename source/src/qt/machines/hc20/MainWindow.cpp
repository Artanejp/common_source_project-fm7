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
#include "emu.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE
Action_Control_HC20::Action_Control_HC20(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	hc20_binds = new Object_Menu_Control_HC20(parent, p);
}

Action_Control_HC20::~Action_Control_HC20(){
	delete hc20_binds;
}

Object_Menu_Control_HC20::Object_Menu_Control_HC20(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_HC20::~Object_Menu_Control_HC20(){
}

void Object_Menu_Control_HC20::set_dipsw(bool flag)
{
	emit sig_dipsw(getValue1(), flag);
}


void META_MainWindow::setupUI_Emu(void)
{
	int i;
	QString tmps;
	menu_Emu_DipSw = new QMenu(menuMachine);
	menu_Emu_DipSw->setObjectName(QString::fromUtf8("menu_DipSw"));

	actionGroup_DipSw = new QActionGroup(this);
	actionGroup_DipSw->setExclusive(false);
	menuMachine->addAction(menu_Emu_DipSw->menuAction());
	for(i = 0; i < 4; i++) {
      	action_Emu_DipSw[i] = new Action_Control_HC20(this, using_flags);
        action_Emu_DipSw[i]->setCheckable(true);
        action_Emu_DipSw[i]->hc20_binds->setValue1(i);
        tmps.number(i + 1);
        tmps = QString::fromUtf8("actionEmu_DipSw") + tmps;
        action_Emu_DipSw[i]->setObjectName(tmps);
		
        if(((1 << i) & config.dipswitch) != 0) action_Emu_DipSw[i]->setChecked(true);
		
		menu_Emu_DipSw->addAction(action_Emu_DipSw[i]);
		actionGroup_DipSw->addAction(action_Emu_DipSw[i]);
		connect(action_Emu_DipSw[i], SIGNAL(toggled(bool)),
				action_Emu_DipSw[i]->hc20_binds, SLOT(set_dipsw(bool)));
		connect(action_Emu_DipSw[i]->hc20_binds, SIGNAL(sig_dipsw(int, bool)),
				this, SLOT(set_dipsw(int, bool)));
		
	}
}

void META_MainWindow::retranslateUi(void)
{
	int i;
	
	retranslateControlMenu(" ",  false);
	retranslateFloppyMenu(0, 1);
	retranslateFloppyMenu(1, 2);
	retranslateCMTMenu();
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
   
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));

	actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));

	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));
  
	menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0));
   // Set Labels
	menu_Emu_DipSw->setTitle(QApplication::translate("MainWindow", "DIP Switches", 0));
	action_Emu_DipSw[0]->setText(QApplication::translate("MainWindow", "Dip Switch 1", 0));
	action_Emu_DipSw[1]->setText(QApplication::translate("MainWindow", "Dip Switch 2", 0));
	action_Emu_DipSw[2]->setText(QApplication::translate("MainWindow", "Dip Switch 3", 0));
	action_Emu_DipSw[3]->setText(QApplication::translate("MainWindow", "Dip Switch 4", 0));
	actionHelp_AboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0));
#ifdef USE_DEBUGGER
	actionDebugger[1]->setText(QApplication::translate("MainWindow", "TF-20 CPU", 0));
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
} // retranslateUi


META_MainWindow::META_MainWindow(USING_FLAGS *p, QWidget *parent) : Ui_MainWindow(p, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



