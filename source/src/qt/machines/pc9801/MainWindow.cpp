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
extern config_t config;

Object_Menu_Control_98::Object_Menu_Control_98(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_98::~Object_Menu_Control_98()
{
}

void Object_Menu_Control_98::do_set_memory_wait(bool flag)
{
	emit sig_set_dipsw(0, flag);
}

void Object_Menu_Control_98::do_set_display_mode(void)
{
	emit sig_display_mode(getValue1());
}


Action_Control_98::Action_Control_98(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	pc98_binds = new Object_Menu_Control_98(parent, p);
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
#ifdef USE_SOUND_TYPE
	menuSoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Card", 0));
	actionSoundDevice[0]->setText(QApplication::translate("MainWindow", "PC-9801-26 (BIOS Enabled)", 0));
	actionSoundDevice[1]->setText(QApplication::translate("MainWindow", "PC-9801-26 (BIOS Disabled)", 0));
	actionSoundDevice[2]->setText(QApplication::translate("MainWindow", "PC-9801-14 (BIOS Enabled)", 0));
	actionSoundDevice[3]->setText(QApplication::translate("MainWindow", "PC-9801-14 (BIOS Disabled)", 0));
	actionSoundDevice[4]->setText(QApplication::translate("MainWindow", "None", 0));
	actionSoundDevice[0]->setToolTip(QApplication::translate("MainWindow", "PC-9801-26 sound board has connected.\nThis uses YAMAHA YM-2203 OPN synthesizer chip.\nOn board BIOS is enabled.", 0));
	actionSoundDevice[1]->setToolTip(QApplication::translate("MainWindow", "PC-9801-26 sound board has connected.\nThis uses YAMAHA YM-2203 OPN synthesizer chip.\nOn board BIOS is disabled.", 0));
	actionSoundDevice[2]->setToolTip(QApplication::translate("MainWindow", "PC-9801-14 sound board has connected.\nThis uses TI TMS3631-RI104 synthesizer chip.\nOn board BIOS is enabled.", 0));
	actionSoundDevice[3]->setToolTip(QApplication::translate("MainWindow", "PC-9801-14 sound board has connected.\nThis uses TI TMS3631-RI104 synthesizer chip.\nOn board BIOS is disabled.", 0));
	actionSoundDevice[4]->setToolTip(QApplication::translate("MainWindow", "None sound devices has connected.", 0));
#endif
#if defined(USE_TAPE)
	retranslateCMTMenu(0);
#endif
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateEmulatorMenu();
#ifdef USE_CPU_TYPE
	menuCpuType->setTitle(QApplication::translate("MainWindow", "CPU Frequency", 0));
# if  defined(_PC98DO)
	actionCpuType[0]->setText(QString::fromUtf8("10/8MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("8/4MHz"));
# elif  defined(_PC98DOPLUS)
	actionCpuType[0]->setText(QString::fromUtf8("16/8MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("8/4MHz"));
# elif  defined(_PC9801E) || defined(_PC9801F) || defined(_PC9801M)
	actionCpuType[0]->setText(QString::fromUtf8("8MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("5MHz"));
# elif  defined(_PC9801VX) || defined(_PC9801VM) || defined(_PC9801XL)
	actionCpuType[0]->setText(QString::fromUtf8("10MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("8MHz"));
# elif  defined(_PC9801RA) || defined(_PC98RL)
	actionCpuType[0]->setText(QString::fromUtf8("20MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("16MHz"));
# endif
#endif
	
#ifdef USE_BOOT_MODE
# ifdef _PC98DO
	menuBootMode->setTitle(QApplication::translate("MainWindow", "Machine Mode", 0));
	menuBootMode->setToolTipsVisible(true);
	actionBootMode[0]->setText(QString::fromUtf8("PC-98"));
	actionBootMode[1]->setText(QString::fromUtf8("N88-V1(S) Mode"));
	actionBootMode[2]->setText(QString::fromUtf8("N88-V1(H) Mode"));	
	actionBootMode[3]->setText(QString::fromUtf8("N88-V2 Mode"));
	actionBootMode[4]->setText(QString::fromUtf8("N Mode (N80 compatible)"));
	actionBootMode[0]->setToolTip(QApplication::translate("MainWindow", "PC-9801 Mode.\nYou can run softwares of PC-9801 series.\nMay be earlier than PC-9801VM.", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("MainWindow", "PC8801 V1(Standard) Mode.\nYou can run softwares of PC-8801/mk2.", 0));
	actionBootMode[2]->setToolTip(QApplication::translate("MainWindow", "PC8801 V1(High Speed) Mode.\nYou can run softwares of PC-8801/mk2 faster.", 0));	
	actionBootMode[3]->setToolTip(QApplication::translate("MainWindow", "PC8801 V2 Mode.\nYou can run only softwares for PC-8801SR or later.", 0));
	actionBootMode[4]->setToolTip(QApplication::translate("MainWindow", "PC8801 N Mode.\nYou can run softwares of PC-8001/mk2.", 0));
# endif
#endif
#ifdef _PC98DO
   	actionMemoryWait->setText(QApplication::translate("MainWindow", "Memory Wait", 0));
	actionMemoryWait->setToolTip(QApplication::translate("MainWindow", "Simulate waiting memory.", 0));
#endif
#if defined(USE_PRINTER)
	actionPrintDevice[1]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MainWindow", "NEC PC-PR201 kanji serial printer.", 0));
#endif	
	retranslateUI_Help();
	// End.
	// Set Labels
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#if defined(_PC9801) || defined(_PC9801E)	
	actionDebugger[1]->setText(QApplication::translate("MainWindow", "PC-80S31K CPU", 0));
	actionDebugger[1]->setVisible(true);
#elif defined(_PC98DO)
	actionDebugger[0]->setText(QApplication::translate("MainWindow", "PC-98 Main CPU", 0));
	actionDebugger[1]->setText(QApplication::translate("MainWindow", "PC-88 Main CPU", 0));
	actionDebugger[2]->setText(QApplication::translate("MainWindow", "PC-88 Sub CPU", 0));
	
	actionDebugger[1]->setVisible(true);
	actionDebugger[2]->setVisible(true);
#endif	
#endif
#ifdef USE_MONITOR_TYPE
	menu_Emu_DisplayMode->setTitle(QApplication::translate("MainWindow", "Display Mode", 0));
	action_Emu_DisplayMode[0]->setText(QApplication::translate("MainWindow", "High Resolution", 0));
	action_Emu_DisplayMode[1]->setText(QApplication::translate("MainWindow", "Standard Resolution", 0));
#endif	
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
	actionMemoryWait = new Action_Control_98(this, using_flags);
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
#ifdef USE_MONITOR_TYPE
   menu_Emu_DisplayMode = new QMenu(menuMachine);
   menu_Emu_DisplayMode->setObjectName(QString::fromUtf8("menu_DisplayMode"));
   
   actionGroup_DisplayMode = new QActionGroup(this);
   actionGroup_DisplayMode->setObjectName(QString::fromUtf8("actionGroup_DisplayMode"));
   actionGroup_DisplayMode->setExclusive(true);
   menuMachine->addAction(menu_Emu_DisplayMode->menuAction());   
   for(int i = 0; i < USE_MONITOR_TYPE; i++) {
	   action_Emu_DisplayMode[i] = new Action_Control_98(this, using_flags);
	   action_Emu_DisplayMode[i]->setCheckable(true);
	   action_Emu_DisplayMode[i]->pc98_binds->setValue1(i);
	   if(i == config.monitor_type) action_Emu_DisplayMode[i]->setChecked(true); // Need to write configure
   }
   action_Emu_DisplayMode[0]->setObjectName(QString::fromUtf8("action_Emu_DisplayMode_High"));
   action_Emu_DisplayMode[1]->setObjectName(QString::fromUtf8("action_Emu_DisplayMode_Standard"));
   for(int i = 0; i < USE_MONITOR_TYPE; i++) {
	   menu_Emu_DisplayMode->addAction(action_Emu_DisplayMode[i]);
	   actionGroup_DisplayMode->addAction(action_Emu_DisplayMode[i]);
	   connect(action_Emu_DisplayMode[i], SIGNAL(triggered()),
			   action_Emu_DisplayMode[i]->pc98_binds, SLOT(do_set_display_mode()));
	   connect(action_Emu_DisplayMode[i]->pc98_binds, SIGNAL(sig_display_mode(int)),
			   this, SLOT(set_monitor_type(int)));
   }
#endif

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



