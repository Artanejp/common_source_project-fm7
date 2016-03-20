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
#include "menu_disk.h"

//QT_BEGIN_NAMESPACE

Object_Menu_Control_98::Object_Menu_Control_98(QObject *parent) : Object_Menu_Control(parent)
{
}

Object_Menu_Control_98::~Object_Menu_Control_98()
{
}

void Object_Menu_Control_98::do_set_memory_wait(bool flag)
{
	emit sig_set_dipsw(0, flag);
}


Action_Control_98::Action_Control_98(QObject *parent) : Action_Control(parent)
{
	pc98_binds = new Object_Menu_Control_98(parent);
	pc98_binds->setValue1(0);
}

Action_Control_98::~Action_Control_98()
{
	delete pc98_binds;
}


void META_MainWindow::retranslateUi(void)
{
	const char *title="";
	retranslateControlMenu(title, false);
	retranslateMachineMenu();
#if defined(USE_FD1)
	retranslateFloppyMenu(0, 1);
#endif
#if defined(USE_FD2)
	retranslateFloppyMenu(1, 2);
#endif
#if defined(USE_FD3)
	retranslateFloppyMenu(2, 3);
#endif
#if defined(USE_FD4)
	retranslateFloppyMenu(3, 4);
#endif
#if defined(USE_FD5)
	retranslateFloppyMenu(4, 5);
#endif
#if defined(USE_FD6)
	retranslateFloppyMenu(5, 6);
#endif
#if defined(_PC9801) || defined(_PC9801E)
   // Drive 3,4
	menu_fds[2]->setTitle(QApplication::translate("MainWindow", "2DD-1", 0));
	menu_fds[3]->setTitle(QApplication::translate("MainWindow", "2DD-2", 0));
   // Drive 5, 6
	menu_fds[4]->setTitle(QApplication::translate("MainWindow", "2D-1", 0));
	menu_fds[5]->setTitle(QApplication::translate("MainWindow", "2D-2", 0));
#elif defined(_PC98DO)
	menu_fds[0]->setTitle(QApplication::translate("MainWindow", "PC98-1", 0));
	menu_fds[1]->setTitle(QApplication::translate("MainWindow", "PC98-2", 0));
	menu_fds[2]->setTitle(QApplication::translate("MainWindow", "PC88-1", 0));
	menu_fds[3]->setTitle(QApplication::translate("MainWindow", "PC88-2", 0));
#endif
#ifdef USE_SOUND_DEVICE_TYPE
	menuSoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Card", 0));
	actionSoundDevice[0]->setText(QApplication::translate("MainWindow", "PC-9801-26 (BIOS Enabled)", 0));
	actionSoundDevice[1]->setText(QApplication::translate("MainWindow", "PC-9801-26 (BIOS Disabled)", 0));
	actionSoundDevice[2]->setText(QApplication::translate("MainWindow", "PC-9801-14 (BIOS Enabled)", 0));
	actionSoundDevice[3]->setText(QApplication::translate("MainWindow", "PC-9801-14 (BIOS Disabled)", 0));
	actionSoundDevice[4]->setText(QApplication::translate("MainWindow", "None", 0));
#endif
#if defined(USE_TAPE)
	retranslateCMTMenu();
#endif
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateEmulatorMenu();
#ifdef USE_CPU_TYPE
	menuCpuType->setTitle("CPU Frequency");
# if  defined(_PC98DO)
	actionCpuType[0]->setText(QString::fromUtf8("10/8MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("5/4MHz"));
# elif  defined(_PC9801E) || defined(_PC9801VM)
	actionCpuType[0]->setText(QString::fromUtf8("8MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("5MHz"));
# endif
#endif
	
#ifdef USE_BOOT_MODE
# ifdef _PC98DO
	menuBootMode->setTitle("Machine Mode");
	actionBootMode[0]->setText(QString::fromUtf8("PC-98"));
	actionBootMode[1]->setText(QString::fromUtf8("N88-V1(S) Mode"));
	actionBootMode[2]->setText(QString::fromUtf8("N88-V1(H) Mode"));	
	actionBootMode[3]->setText(QString::fromUtf8("N88-V2 Mode"));
	actionBootMode[4]->setText(QString::fromUtf8("N Mode (N80 compatible)"));
# endif
#endif
#ifdef _PC98DO
   	actionMemoryWait->setText(QApplication::translate("MainWindow", "Memory Wait", 0));;
#endif
#if defined(USE_PRINTER)
	actionPrintDevice[1]->setText(QString::fromUtf8("PC-PR201"));
#endif	
	retranslateUI_Help();
	// End.
	// Set Labels
	
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
#ifdef USE_CPU_TYPE
	menuCpuType = new QMenu(menuMachine);
	menuCpuType->setObjectName(QString::fromUtf8("menuControl_CpuType"));
	menuMachine->addAction(menuCpuType->menuAction());
	ConfigCPUTypes(2);
#endif
	
#if defined(_PC98DO)
	actionMemoryWait = new Action_Control_98(this);
	actionMemoryWait->setCheckable(true);
	actionMemoryWait->setVisible(true);
	menuMachine->addAction(actionMemoryWait);
	if((config.dipswitch & 0x0001) != 0) actionMemoryWait->setChecked(true);
	connect(actionMemoryWait, SIGNAL(toggled(bool)),
			actionMemoryWait->pc98_binds, SLOT(do_set_memory_wait(bool)));
	connect(actionMemoryWait->pc98_binds, SIGNAL(sig_set_dipsw(int, bool)),
			this, SLOT(set_dipsw(int, bool)));
#endif   
	
#ifdef USE_BOOT_MODE
# if defined(_PC98DO)
	menuBootMode = new QMenu(menuMachine);
	menuBootMode->setObjectName(QString::fromUtf8("menuControl_BootMode"));
	menuMachine->addAction(menuBootMode->menuAction());
	ConfigCPUBootMode(5);
# endif
#endif   
}


META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



