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

void Object_Menu_Control_98::do_set_enable_v30(bool flag)
{
	emit sig_set_dipsw(DIPSWITCH_POSITION_CPU_MODE, flag);
}


void Object_Menu_Control_98::do_set_egc(bool flag)
{
	emit sig_set_dipsw(DIPSWITCH_POSITION_EGC, flag);
	emit sig_emu_update_config();
}

void Object_Menu_Control_98::do_set_gdc_fast(bool flag)
{
	emit sig_set_dipsw(DIPSWITCH_POSITION_GDC_FAST, flag);
	emit sig_emu_update_config();
}

void Object_Menu_Control_98::do_set_ram_512k(bool flag)
{
	emit sig_set_dipsw(DIPSWITCH_POSITION_RAM512K, flag);
	emit sig_emu_update_config();
}

void Object_Menu_Control_98::do_set_init_memsw(bool flag)
{
	emit sig_set_dipsw(DIPSWITCH_POSITION_NOINIT_MEMSW, !flag);
	emit sig_emu_update_config();
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
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(title, false);

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
#if defined(SUPPORT_PC98_OPNA)
	actionSoundDevice[0]->setText(QApplication::translate("MainWindow", "PC-9801-86 (BIOS Enabled)", 0));
	actionSoundDevice[1]->setText(QApplication::translate("MainWindow", "PC-9801-86 (BIOS Disabled)", 0));
#else
	actionSoundDevice[0]->setText(QApplication::translate("MainWindow", "PC-9801-26 (BIOS Enabled)", 0));
	actionSoundDevice[1]->setText(QApplication::translate("MainWindow", "PC-9801-26 (BIOS Disabled)", 0));
#endif	
	actionSoundDevice[2]->setText(QApplication::translate("MainWindow", "PC-9801-14 (BIOS Enabled)", 0));
	actionSoundDevice[3]->setText(QApplication::translate("MainWindow", "PC-9801-14 (BIOS Disabled)", 0));
	actionSoundDevice[4]->setText(QApplication::translate("MainWindow", "None", 0));
#if defined(SUPPORT_PC98_OPNA)
	actionSoundDevice[0]->setToolTip(QApplication::translate("MainWindow", "PC-9801-86 sound board has connected.\nThis uses YAMAHA YM-2608 OPNA synthesizer chip.\nOn board BIOS is enabled.", 0));
	actionSoundDevice[1]->setToolTip(QApplication::translate("MainWindow", "PC-9801-86 sound board has connected.\nThis uses YAMAHA YM-2608 OPNA synthesizer chip.\nOn board BIOS is disabled.", 0));
#else
	actionSoundDevice[0]->setToolTip(QApplication::translate("MainWindow", "PC-9801-26 sound board has connected.\nThis uses YAMAHA YM-2203 OPN synthesizer chip.\nOn board BIOS is enabled.", 0));
	actionSoundDevice[1]->setToolTip(QApplication::translate("MainWindow", "PC-9801-26 sound board has connected.\nThis uses YAMAHA YM-2203 OPN synthesizer chip.\nOn board BIOS is disabled.", 0));
#endif
	actionSoundDevice[2]->setToolTip(QApplication::translate("MainWindow", "PC-9801-14 sound board has connected.\nThis uses TI TMS3631-RI104 synthesizer chip.\nOn board BIOS is enabled.", 0));
	actionSoundDevice[3]->setToolTip(QApplication::translate("MainWindow", "PC-9801-14 sound board has connected.\nThis uses TI TMS3631-RI104 synthesizer chip.\nOn board BIOS is disabled.", 0));
	actionSoundDevice[4]->setToolTip(QApplication::translate("MainWindow", "None sound devices has connected.", 0));
#endif
#if defined(_PC9801RA) || defined(_PC98XL) || defined(_PC98XA) \
	|| defined(_PC98RL) || defined(PC9801VX)
	actionSoundDevice[2]->setVisible(false);
	actionSoundDevice[3]->setVisible(false);
#endif
#if defined(HAS_I286) || defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM)
	actionSUB_V30->setText(QApplication::translate("MainWindow", "Enable V30 SUB CPU(need RESTART).", 0));
	actionSUB_V30->setToolTip(QApplication::translate("MainWindow", "Enable emulation of V30 SUB CPU.\nThis may make emulation speed slower.\nYou must restart emulator after reboot.", 0));
#endif
	
#ifdef USE_CPU_TYPE
	menuCpuType->setTitle(QApplication::translate("MainWindow", "CPU Frequency", 0));
# if  defined(_PC98DO)
	actionCpuType[0]->setText(QString::fromUtf8("V30 10MHz / Z80 8MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("V30 8MHz  / Z80 4MHz"));
# elif  defined(_PC98DOPLUS)
	actionCpuType[0]->setText(QString::fromUtf8("V30 16MHz / Z80 8MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("V30 8MHz  / Z80 4MHz"));
# elif  defined(_PC9801E) || defined(_PC9801F) || defined(_PC9801M)
	actionCpuType[0]->setText(QString::fromUtf8("8086 8MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("8086 5MHz"));
# elif  defined(_PC9801VM)
	actionCpuType[0]->setText(QString::fromUtf8("V30 10MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("V30 8MHz"));
# elif  defined(_PC9801VX) || defined(_PC98XL)
	actionCpuType[0]->setText(QString::fromUtf8("80286 10MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("80286 8MHz"));
	actionCpuType[2]->setText(QString::fromUtf8("V30 8MHz"));
	if((config.dipswitch & ((0x1) << DIPSWITCH_POSITION_CPU_MODE)) == 0) {
		actionCpuType[2]->setEnabled(false);
	}
# elif  defined(_PC9801RA) || defined(_PC98RL)
	// ToDo: PC98RL's display rotate.
	actionCpuType[0]->setText(QString::fromUtf8("80386 20MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("80386 16MHz"));
	actionCpuType[2]->setText(QString::fromUtf8("V30 8MHz"));
	if((config.dipswitch & ((0x1) << DIPSWITCH_POSITION_CPU_MODE)) == 0) {
		actionCpuType[2]->setEnabled(false);
	}
# elif  defined(_PC98XA)
	actionCpuType[0]->setText(QString::fromUtf8("80286 8MHz"));
	actionCpuType[1]->setVisible(false);
	actionCpuType[2]->setText(QString::fromUtf8("V30 8MHz"));
	if((config.dipswitch & ((0x1) << DIPSWITCH_POSITION_CPU_MODE)) == 0) {
		actionCpuType[2]->setEnabled(false);
	}
# endif
#endif
	
	actionRAM_512K->setText(QApplication::translate("MainWindow", "512KB RAM", 0));
	actionRAM_512K->setToolTip(QApplication::translate("MainWindow", "Set lower RAM size to 512KB(not 640KB).\nMaybe for backward compatibility.", 0));
	actionINIT_MEMSW->setText(QApplication::translate("MainWindow", "INIT MEMSW(need RESET)", 0));
	actionINIT_MEMSW->setToolTip(QApplication::translate("MainWindow", "Initialize memory switch.\nEffects after resetting.", 0));
	actionGDC_FAST->setText(QApplication::translate("MainWindow", "FAST GDC", 0));
	actionGDC_FAST->setToolTip(QApplication::translate("MainWindow", "Set GDC clock to 5MHz when checked.\nSet to 2.5MHz wjhen not checked.", 0));
#if defined(SUPPORT_EGC)
	actionEGC->setText(QApplication::translate("MainWindow", "USE EGC", 0));
	actionEGC->setToolTip(QApplication::translate("MainWindow", "Use Enhanced Graphic controller when checked.", 0));
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
	actionPrintDevice[1]->setEnabled(false);
#endif	
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
	actionMonitorType[0]->setText(QApplication::translate("MainWindow", "High Resolution", 0));
	actionMonitorType[1]->setText(QApplication::translate("MainWindow", "Standard Resolution", 0));
#endif	
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
#ifdef USE_CPU_TYPE
	#if defined(HAS_I286) || defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM)
	ConfigCPUTypes(3);
	#else
	ConfigCPUTypes(2);
	#endif
#endif
#if defined(HAS_I286) || defined(HAS_I386) || defined(HAS_I486) || defined(HAS_PENTIUM)
	actionSUB_V30 = new Action_Control_98(this, using_flags);
	actionSUB_V30->setCheckable(true);
	actionSUB_V30->setVisible(true);
	menuMachine->addAction(actionSUB_V30);
	if((config.dipswitch & ((0x1) << DIPSWITCH_POSITION_CPU_MODE)) != 0) actionSUB_V30->setChecked(true); // Emulation with V30
	connect(actionSUB_V30, SIGNAL(toggled(bool)),
			actionSUB_V30->pc98_binds, SLOT(do_set_enable_v30(bool)));
	connect(actionSUB_V30->pc98_binds, SIGNAL(sig_set_dipsw(int, bool)),
			this, SLOT(set_dipsw(int, bool)));
	connect(actionSUB_V30->pc98_binds, SIGNAL(sig_emu_update_config()),
			this, SLOT(do_emu_update_config()));
#endif
   
	actionRAM_512K = new Action_Control_98(this, using_flags);
	actionRAM_512K->setCheckable(true);
	actionRAM_512K->setVisible(true);
	menuMachine->addAction(actionRAM_512K);
	if((config.dipswitch & ((0x1) << DIPSWITCH_POSITION_RAM512K)) != 0) actionRAM_512K->setChecked(true); // DipSW 1-8
	connect(actionRAM_512K, SIGNAL(toggled(bool)),
			actionRAM_512K->pc98_binds, SLOT(do_set_ram_512k(bool)));
	connect(actionRAM_512K->pc98_binds, SIGNAL(sig_set_dipsw(int, bool)),
			this, SLOT(set_dipsw(int, bool)));
	connect(actionRAM_512K->pc98_binds, SIGNAL(sig_emu_update_config()),
			this, SLOT(do_emu_update_config()));
	
	actionINIT_MEMSW = new Action_Control_98(this, using_flags);
	actionINIT_MEMSW->setCheckable(true);
	actionINIT_MEMSW->setVisible(true);
	menuMachine->addAction(actionINIT_MEMSW);
	if((config.dipswitch & ((0x1) << DIPSWITCH_POSITION_NOINIT_MEMSW)) == 0) actionINIT_MEMSW->setChecked(true); // DipSW 1-8
	connect(actionINIT_MEMSW, SIGNAL(toggled(bool)),
			actionINIT_MEMSW->pc98_binds, SLOT(do_set_init_memsw(bool)));
	connect(actionINIT_MEMSW->pc98_binds, SIGNAL(sig_set_dipsw(int, bool)),
			this, SLOT(set_dipsw(int, bool)));
	connect(actionINIT_MEMSW->pc98_binds, SIGNAL(sig_emu_update_config()),
			this, SLOT(do_emu_update_config()));
	
	actionGDC_FAST = new Action_Control_98(this, using_flags);
	actionGDC_FAST->setCheckable(true);
	actionGDC_FAST->setVisible(true);
	menuMachine->addAction(actionGDC_FAST);
	if((config.dipswitch & ((0x1) << DIPSWITCH_POSITION_GDC_FAST)) != 0) actionGDC_FAST->setChecked(true); // DipSW 1-8
	connect(actionGDC_FAST, SIGNAL(toggled(bool)),
			actionGDC_FAST->pc98_binds, SLOT(do_set_gdc_fast(bool)));
	connect(actionGDC_FAST->pc98_binds, SIGNAL(sig_set_dipsw(int, bool)),
			this, SLOT(set_dipsw(int, bool)));
	connect(actionGDC_FAST->pc98_binds, SIGNAL(sig_emu_update_config()),
			this, SLOT(do_emu_update_config()));
	
#if defined(SUPPORT_EGC)
	actionEGC = new Action_Control_98(this, using_flags);
	actionEGC->setCheckable(true);
	actionEGC->setVisible(true);
	menuMachine->addAction(actionEGC);
	if((config.dipswitch & ((0x1) << DIPSWITCH_POSITION_EGC)) != 0) actionEGC->setChecked(true); // DipSW 1-8
	connect(actionEGC, SIGNAL(toggled(bool)),
			actionEGC->pc98_binds, SLOT(do_set_egc(bool)));
	connect(actionEGC->pc98_binds, SIGNAL(sig_set_dipsw(int, bool)),
			this, SLOT(set_dipsw(int, bool)));
	connect(actionEGC->pc98_binds, SIGNAL(sig_emu_update_config()),
			this, SLOT(do_emu_update_config()));
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
	ConfigCPUBootMode(USE_BOOT_MODE);
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



