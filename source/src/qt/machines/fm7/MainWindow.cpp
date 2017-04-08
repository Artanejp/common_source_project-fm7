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

extern config_t config;

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
	retranslateCMTMenu(0);
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
	actionSpecial_Reset->setText(QApplication::translate("MainWindow", "Hot Start(BREAK+RESET)", 0));
	actionSpecial_Reset->setToolTip(QApplication::translate("MainWindow", "Do HOT START.\nReset with pressing BREAK key.", 0));
	
#if defined(USE_PRINTER_TYPE)
	actionPrintDevice[1]->setText(QApplication::translate("MainWindow", "Dempa Joystick with #1", 0));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MainWindow", "Use joystick #1 as DEMPA's joystick.", 0));
	actionPrintDevice[2]->setText(QApplication::translate("MainWindow", "Dempa Joystick with #2", 0));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MainWindow", "Use joystick #2 as DEMPA's joystick.", 0));
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

	menuFrameSkip->setTitle(QApplication::translate("Machine", "Frame skip", 0));
	actionFrameSkip[0]->setText(QApplication::translate("Machine", "None", 0));
	actionFrameSkip[1]->setText(QApplication::translate("Machine", "1 Frame", 0));
	actionFrameSkip[2]->setText(QApplication::translate("Machine", "2 Frames", 0));
	actionFrameSkip[3]->setText(QApplication::translate("Machine", "3 Frames", 0));
#if defined(_FM77AV_VARIANTS)
	actionSyncToHsync->setText(QApplication::translate("Machine", "Sync to HSYNC", 0));
	actionSyncToHsync->setToolTip(QApplication::translate("Machine", "Emulate display syncing to HSYNC.\nExpect to emulate more accurate.", 0));
#endif	
	menuCpuType->setTitle(QApplication::translate("Machine", "CPU Frequency", 0));
	actionCpuType[0]->setText(QString::fromUtf8("2MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("1.2MHz"));
	
	menuBootMode->setTitle(QApplication::translate("Machine", "BOOT Mode", 0));
	menuBootMode->setToolTipsVisible(true);
	actionBootMode[0]->setText(QApplication::translate("Machine", "BASIC", 0));
	actionBootMode[1]->setText(QApplication::translate("Machine", "DOS", 0));	
	actionBootMode[0]->setToolTip(QApplication::translate("Machine", "Boot with F-BASIC.", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("Machine", "DOS boot mode.\nUse for CP/M, FLEX, OS-9, R-DOS and some OSs.", 0));	
#if defined(_FM77_VARIANTS)
	actionBootMode[2]->setVisible(true);
	actionBootMode[2]->setText(QString::fromUtf8("MMR"));
	actionBootMode[2]->setToolTip(QApplication::translate("Machine", "MMR boot mode.\nThis is FM-77 feature and I don't know about this.", 0));
#elif defined(_FM8)
	actionBootMode[2]->setText(QApplication::translate("Machine", "Bubble Casette", 0));
	actionBootMode[3]->setText(QApplication::translate("Machine", "8Inch FD", 0));
	actionBootMode[2]->setToolTip(QApplication::translate("Machine", "Boot for bubble casette.\nYou must install boot rom for this.", 0));
	actionBootMode[3]->setToolTip(QApplication::translate("Machine", "Boot for 8inch floppy disk.\nYou must install boot rom for this.", 0));
	
	actionBootMode[2]->setVisible(true);
	actionBootMode[3]->setVisible(true);
#elif defined(_FM7) || defined(_FMNEW7)	
	actionBootMode[2]->setVisible(false);
#endif

#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	actionKanjiRom->setText(QApplication::translate("Machine", "Kanji ROM(Need restart)", 0));
	actionKanjiRom->setToolTip(QApplication::translate("Machine", "Connect KANJI ROM.Need restart emulator if changed.", 0));
#endif	
#if defined(_FM8)
	actionRamProtect->setText(QApplication::translate("Machine", "BANK PROTECT($FD0F/hack)", 0));
	actionRamProtect->setToolTip(QApplication::translate("Machine", "Protect bank setting if checked.\nUnchecked to simulate FM-7.\nUseful for some software tranferring to URA-RAM.", 0));
#elif defined(_FM7) || defined(_FMNEW7)	
	actionCycleSteal->setText(QApplication::translate("Machine", "Cycle Steal(hack)", 0));
	actionCycleSteal->setToolTip(QApplication::translate("Machine", "Enabling cycle steal to be faster drawing.\nThis is hack for FM-7.", 0));
#else							  
	actionCycleSteal->setText(QApplication::translate("Machine", "Cycle Steal", 0));
	actionCycleSteal->setToolTip(QApplication::translate("Machine", "Enabling cycle steal to be faster drawing.", 0));
#endif							  
	menuSoundDevice->setTitle(QApplication::translate("Machine", "Sound Boards", 0));
#if defined(USE_SOUND_TYPE)
# if defined(_FM8)
	actionSoundDevice[0]->setVisible(true);
	actionSoundDevice[1]->setVisible(true);
	actionSoundDevice[0]->setText(QApplication::translate("Machine", "Beep Only", 0));
	actionSoundDevice[0]->setToolTip(QApplication::translate("Machine", "Use only buzzer as sound device.", 0));
	actionSoundDevice[1]->setText(QApplication::translate("Machine", "PSG (hack)", 0));
	actionSoundDevice[1]->setToolTip(QApplication::translate("Machine", "Set FM-7 compatible PSG.\nThis emulates extra PSG board made by third party.", 0));
# elif defined(_FM77AV_VARIANTS)
	actionSoundDevice[0]->setVisible(false);
	actionSoundDevice[2]->setVisible(false);
	actionSoundDevice[4]->setVisible(false);
	actionSoundDevice[6]->setVisible(false);
	actionSoundDevice[1]->setText(QApplication::translate("Machine", "OPN", 0));
	actionSoundDevice[3]->setText(QApplication::translate("Machine", "OPN+WHG", 0));
	actionSoundDevice[5]->setText(QApplication::translate("Machine", "OPN+THG", 0));
	actionSoundDevice[7]->setText(QApplication::translate("Machine", "OPN+WHG+THG", 0));
	
	actionSoundDevice[1]->setToolTip(QApplication::translate("Machine", "Using only default FM synthesizer board.", 0));
	actionSoundDevice[3]->setToolTip(QApplication::translate("Machine", "Using default FM synthesizer board\nand WHG second FM synthesizer board.", 0));
	actionSoundDevice[5]->setToolTip(QApplication::translate("Machine", "Using default FM synthesizer board\nand THG third FM synthesizer board.", 0));
	actionSoundDevice[7]->setToolTip(QApplication::translate("Machine", "Using default FM synthesizer board\nand WHG second FM synthesizer board\nand THG third FM synthesizer board.", 0));
# else
	actionSoundDevice[0]->setText(QApplication::translate("Machine", "PSG", 0));
	actionSoundDevice[1]->setText(QApplication::translate("Machine", "PSG+OPN", 0));
	actionSoundDevice[2]->setText(QApplication::translate("Machine", "PSG+WHG", 0));
	actionSoundDevice[3]->setText(QApplication::translate("Machine", "PSG+OPN+WHG", 0));
	actionSoundDevice[4]->setText(QApplication::translate("Machine", "PSG+THG", 0));
	actionSoundDevice[5]->setText(QApplication::translate("Machine", "PSG+OPN+THG", 0));
	actionSoundDevice[6]->setText(QApplication::translate("Machine", "PSG+WHG+THG", 0));
	actionSoundDevice[7]->setText(QApplication::translate("Machine", "PSG+OPN+WHG+THG", 0));
	
	actionSoundDevice[0]->setToolTip(QApplication::translate("Machine", "Using only default PSG.", 0));
	actionSoundDevice[1]->setToolTip(QApplication::translate("Machine", "Using default PSG \nand Fujitsu's FM synthesizer board.", 0));
	actionSoundDevice[2]->setToolTip(QApplication::translate("Machine", "Using default PSG \nand WHG second FM synthesizer board.", 0));
	actionSoundDevice[3]->setToolTip(QApplication::translate("Machine", "Using default PSG \nand Fujitsu's FM synthesizer board\nand WHG second FM synthesizer board.", 0));
	actionSoundDevice[4]->setToolTip(QApplication::translate("Machine", "Using default PSG \nand THG third FM synthesizer board.", 0));
	actionSoundDevice[5]->setToolTip(QApplication::translate("Machine", "Using default PSG \nand Fujitsu's FM synthesizer board\nand THG third FM synthesizer board.", 0));
	actionSoundDevice[6]->setToolTip(QApplication::translate("Machine", "Using default PSG \nand WHG 2nd FM sythesizer board\nand THG third FM synthesizer board.", 0));
	actionSoundDevice[7]->setToolTip(QApplication::translate("Machine", "Using default PSG \nand Fujitsu's FM synthesizer board\nand WHG second FM synthesizer board\nand THG third FM synthesizer board.", 0));
# endif
#endif
#if !defined(_FM8)
# if defined(USE_DEVICE_TYPE)
	menuDeviceType->setTitle(QApplication::translate("Machine", "Mouse", 0));
	actionDeviceType[0]->setText(QApplication::translate("Machine", "none", 0));
	actionDeviceType[1]->setText(QApplication::translate("Machine", "JS port1", 0));
	actionDeviceType[1]->setToolTip(QApplication::translate("Machine", "Connect mouse to JOYSTICK port #1.", 0));
	actionDeviceType[2]->setText(QApplication::translate("Machine", "JS port2", 0));
	actionDeviceType[2]->setToolTip(QApplication::translate("Machine", "Connect mouse to JOYSTICK port #2.", 0));
# endif	
# if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	actionExtRam->setText(QApplication::translate("Machine", "Use Extra RAM (Need reboot)", 0));
	actionExtRam->setToolTip(QApplication::translate("Machine", "Using extra ram block.\nNeed to reboot if changed.", 0));
# endif
#endif
#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	action_320kFloppy->setText(QApplication::translate("Machine", "Connect 320KB FDD(Need Restart)", 0));
	action_320kFloppy->setToolTip(QApplication::translate("Machine", "Connect 2D floppy drive.\nNeed to restart emulator if changed.", 0));
#endif	
#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
	action_1MFloppy->setText(QApplication::translate("Machine", "Connect 1MB FDD(Need Restart)", 0));
	action_1MFloppy->setToolTip(QApplication::translate("Machine", "**Note: This option still not implemented**\nConnect 2HD (or 8inch) floppy drive.\nNeed to restart emulator if changed.\n", 0));
#endif
	menuAuto5_8Key->setTitle(QApplication::translate("Machine", "Auto Stop Ten Key (hack)", 0));
	action_Neither_5_or_8key->setText(QApplication::translate("Machine", "None used.", 0));
	action_Auto_5key->setText(QApplication::translate("Machine", "Use 5", 0));
	action_Auto_5key->setToolTip(QApplication::translate("Machine", "Use '5' key to stop with some GAMES.\nUseful for games using '2 4 6 8' to move character.", 0));
	action_Auto_8key->setText(QApplication::translate("Machine", "Use 8", 0));
	action_Auto_8key->setToolTip(QApplication::translate("Machine", "Use '8' key to stop with some GAMES.\nUseful for games using '1 2 3 5' to move character.", 0));
	// Set Labels
	menuMachine->setToolTipsVisible(true);
	menuAuto5_8Key->setToolTipsVisible(true);
#if !defined(_FM8)
# if defined(USE_DEVICE_TYPE)
	menuDeviceType->setToolTipsVisible(true);
# endif
#endif
	menuCpuType->setToolTipsVisible(true);
	menuBootMode->setToolTipsVisible(true);
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
		if((uint32_t)i == skip) actionFrameSkip[i]->setChecked(true);
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


META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



