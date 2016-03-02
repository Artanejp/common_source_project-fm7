/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QtCore/QVariant>
#include <QtGui>
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"
#include "sound_dialog.h"

#include "../../vm/fm7/fm7_common.h"

//QT_BEGIN_NAMESPACE


Object_Menu_Control_7::Object_Menu_Control_7(QObject *parent) : Object_Menu_Control(parent)
{
}

Object_Menu_Control_7::~Object_Menu_Control_7()
{
}

#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
void Object_Menu_Control_7::do_set_kanji_rom(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_CONNECT_KANJIROM;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_CONNECT_KANJIROM;
	}
}
#endif

#if defined(_FM8)
void Object_Menu_Control_7::do_set_protect_ram(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_FM8_PROTECT_FD0F;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_FM8_PROTECT_FD0F;
	}
	emit sig_emu_update_config();
}
#else
void Object_Menu_Control_7::do_set_cyclesteal(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_CYCLESTEAL;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_CYCLESTEAL;
	}
	emit sig_emu_update_config();
}
#endif

#if defined(_FM77AV_VARIANTS)
void Object_Menu_Control_7::do_set_hsync(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_SYNC_TO_HSYNC;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_SYNC_TO_HSYNC;
	}
}
#endif

void Action_Control_7::do_set_frameskip()
{
	config.dipswitch &= 0xcfffffff;
	config.dipswitch |= ((fm7_binds->getValue1() & 3) << 28);
}

Action_Control_7::Action_Control_7(QObject *parent) : Action_Control(parent)
{
	fm7_binds = new Object_Menu_Control_7(parent);
	fm7_binds->setValue1(0);
}

Action_Control_7::~Action_Control_7()
{
	delete fm7_binds;
}



void META_MainWindow::do_set_extram(bool flag)
{
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77AV20) || defined(_FM77_VARIANTS)
	if(flag) {	
		config.dipswitch |= FM7_DIPSW_EXTRAM;
	} else {
		config.dipswitch &= ~FM7_DIPSW_EXTRAM;
	}
#else
# if defined(_FM77AV_VARIANTS)
	if(flag) {	
		config.dipswitch |= FM7_DIPSW_EXTRAM_AV;
	} else {
		config.dipswitch &= ~FM7_DIPSW_EXTRAM_AV;
	}
# endif
#endif
}

void META_MainWindow::retranslateVolumeLabels(Ui_SoundDialog *p)
{
}

void META_MainWindow::retranslateUi(void)
{
	
	retranslateControlMenu("HOT START", true);
	retranslateFloppyMenu(0, 0);
	retranslateFloppyMenu(1, 1);
	retranslateCMTMenu();
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
	
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
	
	actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0));
#if defined(USE_PRINTER_TYPE)
	actionPrintDevice[1]->setText(QApplication::translate("MainWindow", "Dempa Joystick with #1", 0));
	actionPrintDevice[2]->setText(QApplication::translate("MainWindow", "Dempa Joystick with #2", 0));
#endif
#ifdef USE_DEBUGGER
	actionDebugger_1->setText(QApplication::translate("MainWindow", "Debug Main CPU", 0));
	actionDebugger_2->setText(QApplication::translate("MainWindow", "Debug Sub  CPU", 0));
	actionDebugger_3->setVisible(false);
#endif	
	//	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0));
	//      actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0));
	// 
	menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0));
	// FM-7 Specified

	menuFrameSkip->setTitle(QApplication::translate("MainWindow", "Frame skip", 0));
	actionFrameSkip[0]->setText(QApplication::translate("MainWindow", "None", 0));
	actionFrameSkip[1]->setText(QApplication::translate("MainWindow", "1 Frame", 0));
	actionFrameSkip[2]->setText(QApplication::translate("MainWindow", "2 Frames", 0));
	actionFrameSkip[3]->setText(QApplication::translate("MainWindow", "3 Frames", 0));
#if defined(_FM77AV_VARIANTS)
	actionSyncToHsync->setText(QApplication::translate("MainWindow", "Sync to HSYNC", 0));
#endif	
	menuCpuType->setTitle(QApplication::translate("MainWindow", "CPU Frequency", 0));
	actionCpuType[0]->setText(QString::fromUtf8("2MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("1.2MHz"));
	
	menuBootMode->setTitle("BOOT Mode");
	actionBootMode[0]->setText(QString::fromUtf8("BASIC"));
	actionBootMode[1]->setText(QString::fromUtf8("DOS"));	
#if defined(_FM77_VARIANTS)
	actionBootMode[2]->setVisible(true);
	actionBootMode[2]->setText(QString::fromUtf8("MMR"));
#elif defined(_FM8)
	actionBootMode[2]->setText(QApplication::translate("MainWindow", "Bubble Casette", 0));
	actionBootMode[3]->setText(QApplication::translate("MainWindow", "8Inch FD", 0));
	
	actionBootMode[2]->setVisible(true);
	actionBootMode[3]->setVisible(true);
#elif defined(_FM7) || defined(_FMNEW7)	
	actionBootMode[2]->setVisible(false);
#endif

#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	actionKanjiRom->setText(QApplication::translate("MainWindow", "Kanji ROM(Need restart)", 0));
#endif	
#if defined(_FM8)
	actionRamProtect->setText(QString::fromUtf8("BANK PROTECT($FD0F/hack)"));
#elif defined(_FM7) || defined(_FMNEW7)	
	actionCycleSteal->setText(QString::fromUtf8("Cycle Steal(hack)"));
#else							  
	actionCycleSteal->setText(QString::fromUtf8("Cycle Steal"));
#endif							  
	menuSoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Boards", 0));
#if defined(USE_SOUND_DEVICE_TYPE)
# if defined(_FM8)
	actionSoundDevice[0]->setVisible(true);
	actionSoundDevice[1]->setVisible(true);
	actionSoundDevice[0]->setText(QString::fromUtf8("Beep Only"));
	actionSoundDevice[1]->setText(QString::fromUtf8("PSG (hack)"));
# elif defined(_FM77AV_VARIANTS)
	actionSoundDevice[0]->setVisible(false);
	actionSoundDevice[2]->setVisible(false);
	actionSoundDevice[4]->setVisible(false);
	actionSoundDevice[6]->setVisible(false);
	actionSoundDevice[1]->setText(QString::fromUtf8("OPN"));
	actionSoundDevice[3]->setText(QString::fromUtf8("OPN+WHG"));
	actionSoundDevice[5]->setText(QString::fromUtf8("OPN+THG"));
	actionSoundDevice[7]->setText(QString::fromUtf8("OPN+WHG+THG"));
# else
	actionSoundDevice[0]->setText(QString::fromUtf8("PSG"));
	actionSoundDevice[1]->setText(QString::fromUtf8("PSG+OPN"));
	actionSoundDevice[2]->setText(QString::fromUtf8("PSG+WHG"));
	actionSoundDevice[3]->setText(QString::fromUtf8("PSG+OPN+WHG"));
	actionSoundDevice[4]->setText(QString::fromUtf8("PSG+THG"));
	actionSoundDevice[5]->setText(QString::fromUtf8("PSG+OPN+THG"));
	actionSoundDevice[6]->setText(QString::fromUtf8("PSG+WHG+THG"));
	actionSoundDevice[7]->setText(QString::fromUtf8("PSG+OPN+WHG+THG"));
# endif
#endif
#if !defined(_FM8)
# if defined(USE_DEVICE_TYPE)
	menuDeviceType->setTitle(QApplication::translate("MainWindow", "Mouse", 0));
	actionDeviceType[0]->setText(QApplication::translate("MainWindow", "none", 0));
	actionDeviceType[1]->setText(QApplication::translate("MainWindow", "JS port1", 0));
	actionDeviceType[2]->setText(QApplication::translate("MainWindow", "JS port2", 0));
# endif	
# if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	actionExtRam->setText(QString::fromUtf8("Use Extra RAM (Need reboot)"));
# endif
#endif							  
	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));
	
	// Set Labels
  
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
	int i;
	uint32_t skip;
	menuFrameSkip = new QMenu(menuMachine);
	menuFrameSkip->setObjectName(QString::fromUtf8("menuControl_FrameSkip"));
	actionGroup_FrameSkip = new QActionGroup(this);
	actionGroup_FrameSkip->setExclusive(true);
	skip = (config.dipswitch >> 28) & 3;
	for(i = 0; i < 4; i++) {
		actionFrameSkip[i] = new Action_Control_7(this);
		actionFrameSkip[i]->setCheckable(true);
		actionFrameSkip[i]->setVisible(true);
		actionFrameSkip[i]->fm7_binds->setValue1(i);
		actionGroup_FrameSkip->addAction(actionFrameSkip[i]);
		menuFrameSkip->addAction(actionFrameSkip[i]);
		if(i == skip) actionFrameSkip[i]->setChecked(true);
		connect(actionFrameSkip[i], SIGNAL(triggered()), actionFrameSkip[i], SLOT(do_set_frameskip()));
	}
	menuMachine->addAction(menuFrameSkip->menuAction());
	
	menuCpuType = new QMenu(menuMachine);
	menuCpuType->setObjectName(QString::fromUtf8("menuControl_CpuType"));
	menuMachine->addAction(menuCpuType->menuAction());
	ConfigCPUTypes(2);
	menuBootMode = new QMenu(menuMachine);
	menuBootMode->setObjectName(QString::fromUtf8("menuControl_BootMode"));
	menuMachine->addAction(menuBootMode->menuAction());

#if defined(_FM8)
	ConfigCPUBootMode(4);
#elif defined(_FM77AV_VARIANTS)
	ConfigCPUBootMode(2);
#else
	ConfigCPUBootMode(3);
#endif
#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	actionKanjiRom = new Action_Control_7(this);
	menuMachine->addAction(actionKanjiRom);
	actionKanjiRom->setCheckable(true);
	actionKanjiRom->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_CONNECT_KANJIROM) != 0) actionKanjiRom->setChecked(true);
	connect(actionKanjiRom, SIGNAL(toggled(bool)),
		 actionKanjiRom->fm7_binds, SLOT(do_set_kanji_rom(bool)));
#endif

#if defined(_FM8)
	actionRamProtect = new Action_Control_7(this);
	menuMachine->addAction(actionRamProtect);
	actionRamProtect->setCheckable(true);
	actionRamProtect->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_FM8_PROTECT_FD0F) != 0) actionRamProtect->setChecked(true);
	connect(actionRamProtect, SIGNAL(toggled(bool)),
			actionRamProtect->fm7_binds, SLOT(do_set_protect_ram(bool)));
	connect(actionRamProtect->fm7_binds, SIGNAL(sig_emu_update_config()),
			this, SLOT(do_emu_update_config()));
#else	
	actionCycleSteal = new Action_Control_7(this);
	menuMachine->addAction(actionCycleSteal);
	actionCycleSteal->setCheckable(true);
	actionCycleSteal->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_CYCLESTEAL) != 0) actionCycleSteal->setChecked(true);
	connect(actionCycleSteal, SIGNAL(toggled(bool)),
		 actionCycleSteal->fm7_binds, SLOT(do_set_cyclesteal(bool)));
	connect(actionCycleSteal->fm7_binds, SIGNAL(sig_emu_update_config()),
			this, SLOT(do_emu_update_config()));
#endif	
#if defined(_FM77AV_VARIANTS)	
	actionSyncToHsync = new Action_Control_7(this);	
	menuMachine->addAction(actionSyncToHsync);
	actionSyncToHsync->setCheckable(true);
	actionSyncToHsync->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) != 0) actionSyncToHsync->setChecked(true);
	connect(actionSyncToHsync, SIGNAL(toggled(bool)),
			actionSyncToHsync->fm7_binds, SLOT(do_set_hsync(bool)));
#endif
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	actionExtRam = new Action_Control_7(this);
	menuMachine->addAction(actionExtRam);
	actionExtRam->setCheckable(true);
	actionExtRam->setVisible(true);
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77_VARIANTS)
	if((config.dipswitch & FM7_DIPSW_EXTRAM) != 0) actionExtRam->setChecked(true);
# elif defined(_FM77AV_VARIANTS)
	if((config.dipswitch & FM7_DIPSW_EXTRAM_AV) != 0) actionExtRam->setChecked(true);
# endif   
	connect(actionExtRam, SIGNAL(toggled(bool)),
			this, SLOT(do_set_extram(bool)));
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



