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


Object_Menu_Control_7::Object_Menu_Control_7(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
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

#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
void Object_Menu_Control_7::do_set_320kFloppy(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_CONNECT_320KFDC;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_CONNECT_320KFDC;
	}
}
#endif

#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
void Object_Menu_Control_7::do_set_1MFloppy(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_CONNECT_1MFDC;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_CONNECT_1MFDC;
	}
}
#endif


void Object_Menu_Control_7::do_set_autokey_5_8(void)
{
	int val = getValue1();
	switch(val) {
	case 0: // Not Auto Key:
		config.dipswitch = config.dipswitch & ~(FM7_DIPSW_SELECT_5_OR_8KEY | FM7_DIPSW_AUTO_5_OR_8KEY);
		break;
	case 1: // Auto 5
		config.dipswitch = config.dipswitch | (FM7_DIPSW_SELECT_5_OR_8KEY | FM7_DIPSW_AUTO_5_OR_8KEY);
		break;
	case 2: // Auto 8
		config.dipswitch = (config.dipswitch | FM7_DIPSW_AUTO_5_OR_8KEY) & ~FM7_DIPSW_SELECT_5_OR_8KEY;
		break;
	default:// Not Auto Key:
		config.dipswitch = config.dipswitch & ~(FM7_DIPSW_SELECT_5_OR_8KEY | FM7_DIPSW_AUTO_5_OR_8KEY);
		break;
	}
}




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

Action_Control_7::Action_Control_7(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	fm7_binds = new Object_Menu_Control_7(parent, p);
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
	
	retranslateControlMenu("Hot Start (BREAK+RESET)", true);
	retranslateFloppyMenu(0, 0);
	retranslateFloppyMenu(1, 1);
	retranslateCMTMenu();
#if defined(USE_BUBBLE1)
	retranslateBubbleMenu(0, 1);
#endif	
#if defined(USE_BUBBLE2)
	retranslateBubbleMenu(1, 2);
#endif
	retranslateSoundMenu();
	retranslateScreenMenu();
	
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
	
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
	
#if defined(USE_PRINTER_TYPE)
	actionPrintDevice[1]->setText(QApplication::translate("MainWindow", "Dempa Joystick with #1", 0));
	actionPrintDevice[2]->setText(QApplication::translate("MainWindow", "Dempa Joystick with #2", 0));
#endif
#ifdef USE_DEBUGGER
	actionDebugger[0]->setText(QApplication::translate("MainWindow", "Main CPU", 0));
	actionDebugger[1]->setText(QApplication::translate("MainWindow", "Sub  CPU", 0));
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
	
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif	
	//	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0));
	//      actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0));
	// 
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
#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	action_320kFloppy->setText(QApplication::translate("MainWindow", "Connect 320KB FDD(Need Restart)", 0));
#endif	
#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
	action_1MFloppy->setText(QApplication::translate("MainWindow", "Connect 1MB FDD(Need Restart)", 0));
#endif
	menuAuto5_8Key->setTitle(QApplication::translate("MainWindow", "Auto Stop Ten Key (hack)", 0));
	action_Neither_5_or_8key->setText(QApplication::translate("MainWindow", "None used.", 0));
	action_Auto_5key->setText(QApplication::translate("MainWindow", "Use 5", 0));
	action_Auto_8key->setText(QApplication::translate("MainWindow", "Use 8", 0));
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
		actionFrameSkip[i] = new Action_Control_7(this, using_flags);
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
	actionKanjiRom = new Action_Control_7(this, using_flags);
	menuMachine->addAction(actionKanjiRom);
	actionKanjiRom->setCheckable(true);
	actionKanjiRom->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_CONNECT_KANJIROM) != 0) actionKanjiRom->setChecked(true);
	connect(actionKanjiRom, SIGNAL(toggled(bool)),
		 actionKanjiRom->fm7_binds, SLOT(do_set_kanji_rom(bool)));
#endif

#if defined(_FM8)
	actionRamProtect = new Action_Control_7(this, using_flags);
	menuMachine->addAction(actionRamProtect);
	actionRamProtect->setCheckable(true);
	actionRamProtect->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_FM8_PROTECT_FD0F) != 0) actionRamProtect->setChecked(true);
	connect(actionRamProtect, SIGNAL(toggled(bool)),
			actionRamProtect->fm7_binds, SLOT(do_set_protect_ram(bool)));
	connect(actionRamProtect->fm7_binds, SIGNAL(sig_emu_update_config()),
			this, SLOT(do_emu_update_config()));
#else	
	actionCycleSteal = new Action_Control_7(this, using_flags);
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
	actionSyncToHsync = new Action_Control_7(this, using_flags);	
	menuMachine->addAction(actionSyncToHsync);
	actionSyncToHsync->setCheckable(true);
	actionSyncToHsync->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) != 0) actionSyncToHsync->setChecked(true);
	connect(actionSyncToHsync, SIGNAL(toggled(bool)),
			actionSyncToHsync->fm7_binds, SLOT(do_set_hsync(bool)));
#endif
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
	actionExtRam = new Action_Control_7(this, using_flags);
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
# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	action_320kFloppy = new Action_Control_7(this, using_flags);	
	menuMachine->addAction(action_320kFloppy);
	action_320kFloppy->setCheckable(true);
	action_320kFloppy->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_CONNECT_320KFDC) != 0) action_320kFloppy->setChecked(true);
	connect(action_320kFloppy, SIGNAL(toggled(bool)),
			action_320kFloppy->fm7_binds, SLOT(do_set_320kFloppy(bool)));
# endif	
# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
	action_1MFloppy = new Action_Control_7(this, using_flags);	
	menuMachine->addAction(action_1MFloppy);
	action_1MFloppy->setCheckable(true);
	action_1MFloppy->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_CONNECT_1MFDC) != 0) action_1MFloppy->setChecked(true);
	connect(action_1MFloppy, SIGNAL(toggled(bool)),
			action_1MFloppy->fm7_binds, SLOT(do_set_1MFloppy(bool)));
#endif
	uint32_t tmpv = config.dipswitch & (FM7_DIPSW_SELECT_5_OR_8KEY | FM7_DIPSW_AUTO_5_OR_8KEY);
	
	menuAuto5_8Key = new QMenu(menuMachine);
	menuAuto5_8Key->setObjectName(QString::fromUtf8("menuControl_Auto5_8Key"));
	menuMachine->addAction(menuAuto5_8Key->menuAction());
	
	actionGroup_Auto_5_8key = new QActionGroup(this);
	actionGroup_Auto_5_8key->setExclusive(true);
	
	action_Neither_5_or_8key = new Action_Control_7(this, using_flags);
	action_Neither_5_or_8key->setCheckable(true);
	action_Neither_5_or_8key->setVisible(true);
	action_Neither_5_or_8key->fm7_binds->setValue1(0);
	actionGroup_Auto_5_8key->addAction(action_Neither_5_or_8key);
	menuAuto5_8Key->addAction(action_Neither_5_or_8key);
	if((tmpv &  FM7_DIPSW_AUTO_5_OR_8KEY) == 0) action_Neither_5_or_8key->setChecked(true);
	connect(action_Neither_5_or_8key, SIGNAL(triggered()),
			action_Neither_5_or_8key->fm7_binds, SLOT(do_set_autokey_5_8()));
	
	action_Auto_5key = new Action_Control_7(this, using_flags);
	action_Auto_5key->setCheckable(true);
	action_Auto_5key->setVisible(true);
	action_Auto_5key->fm7_binds->setValue1(1);
	actionGroup_Auto_5_8key->addAction(action_Auto_5key);
	menuAuto5_8Key->addAction(action_Auto_5key);
	if((tmpv &  FM7_DIPSW_AUTO_5_OR_8KEY) != 0) {
		if((tmpv &  FM7_DIPSW_SELECT_5_OR_8KEY) != 0){
			action_Auto_5key->setChecked(true);
		}
	}
	connect(action_Auto_5key, SIGNAL(triggered()),
			action_Auto_5key->fm7_binds, SLOT(do_set_autokey_5_8()));

	action_Auto_8key = new Action_Control_7(this, using_flags);
	action_Auto_8key->setCheckable(true);
	action_Auto_8key->setVisible(true);
	action_Auto_8key->fm7_binds->setValue1(2);
	actionGroup_Auto_5_8key->addAction(action_Auto_8key);
	menuAuto5_8Key->addAction(action_Auto_8key);
	if((tmpv &  FM7_DIPSW_AUTO_5_OR_8KEY) != 0) {
		if((tmpv &  FM7_DIPSW_SELECT_5_OR_8KEY) == 0){
			action_Auto_8key->setChecked(true);
		}
	}
	connect(action_Auto_8key, SIGNAL(triggered()),
			action_Auto_8key->fm7_binds, SLOT(do_set_autokey_5_8()));
}


META_MainWindow::META_MainWindow(USING_FLAGS *p, QWidget *parent) : Ui_MainWindow(p, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



