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

}

void META_MainWindow::retranslateUi(void)
{
	retranslateControlMenu(" ",  false);
	retranslateFloppyMenu(0, 1);
	retranslateFloppyMenu(1, 2);
	retranslateFloppyMenu(2, 3);
	retranslateFloppyMenu(3, 4);
	retranslateCMTMenu(0);
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();

#if defined(_MZ80A) || defined(_MZ80K)	
	action_Emu_DipSw->setText(QApplication::translate("MachineMZ80K", "PCG-8000", 0));
	action_Emu_DipSw->setToolTip(QApplication::translate("MachineMZ80K", "HAL laboratory PCG-8000 PCG.", 0));
#elif defined(_MZ1200)
	action_Emu_DipSw->setText(QApplication::translate("MachineMZ80K", "PCG-1200", 0));
	action_Emu_DipSw->setToolTip(QApplication::translate("MachineMZ80K", "HAL laboratory PCG-1200 PCG.", 0));
#endif
	actionPrintDevice[1]->setText(QString::fromUtf8("MZ-1P17"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineMZ80K", "Sharp MZ-1P17 kanji thermal printer.", 0));
	actionPrintDevice[2]->setText(QString::fromUtf8("DUMMY"));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MachineMZ80K", "DUMMY printer.", 0));
	this->setWindowTitle(QApplication::translate("MachineMZ80K", "MachineMZ80K", 0));
#if defined(USE_DRIVE_TYPE)
	menuDriveType->setTitle(QApplication::translate("MachineMZ80K", "Floppy Type", 0));
	actionDriveType[0]->setText(QApplication::translate("MachineMZ80K", "2D", 0));
	actionDriveType[1]->setText(QApplication::translate("MachineMZ80K", "2DD", 0));
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



