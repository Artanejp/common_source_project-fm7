/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QApplication>
#include <QtCore/QVariant>
#include <QtGui>
#include <QMenu>

#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"
#include "sound_dialog.h"

#include "../../vm/fm7/fm7_common.h"
#include "menu_disk.h"
//QT_BEGIN_NAMESPACE

extern config_t config;

Object_Menu_Control_7::Object_Menu_Control_7(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_7::~Object_Menu_Control_7()
{
}

#if defined(WITH_Z80)
void Object_Menu_Control_7::do_set_z80card_on(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_Z80CARD_ON;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_Z80CARD_ON;
	}
}

void Object_Menu_Control_7::do_set_z80_irq(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_Z80_IRQ_ON;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_Z80_IRQ_ON;
	}
	emit sig_emu_update_config();
}

void Object_Menu_Control_7::do_set_z80_firq(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_Z80_FIRQ_ON;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_Z80_FIRQ_ON;
	}
	emit sig_emu_update_config();
}

void Object_Menu_Control_7::do_set_z80_nmi(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_Z80_NMI_ON;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_Z80_NMI_ON;
	}
	emit sig_emu_update_config();
}
#endif
void Object_Menu_Control_7::do_set_uart(bool flag)
{
	uint32_t nval;
	int num = getValue1();
	switch(num) {
	case 0:
		nval = FM7_DIPSW_RS232C_ON;
		break;
	case 1:
		nval = FM7_DIPSW_MODEM_ON;
		break;
	case 2:
		nval = FM7_DIPSW_MIDI_ON;
		break;
	default:
		return;
		break;
	}
	if(flag) {
		config.dipswitch = config.dipswitch | nval;
	} else {
		nval = (uint32_t)(~nval);
		config.dipswitch = config.dipswitch & nval;
	}
}

#if defined(CAPABLE_JCOMMCARD)
void Object_Menu_Control_7::do_set_jcommcard(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_JSUBCARD_ON;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_JSUBCARD_ON;
	}
}
#endif

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

#if defined(HAS_2HD)
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
#if defined(USE_GREEN_DISPLAY)
void Object_Menu_Control_7::do_set_green_display(bool flag)
{
#if defined(USE_MONITOR_TYPE)
	if(flag) {
		config.monitor_type = FM7_MONITOR_GREEN;
	} else {
		config.monitor_type = FM7_MONITOR_STANDARD;
	}
#endif

}
#endif
void Object_Menu_Control_7::do_set_hsync(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_SYNC_TO_HSYNC;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_SYNC_TO_HSYNC;
	}
}

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
#elif defined(_FM77AV_VARIANTS)
	if(flag) {	
		config.dipswitch |= FM7_DIPSW_EXTRAM_AV;
	} else {
		config.dipswitch &= ~FM7_DIPSW_EXTRAM_AV;
	}
#endif
}
#if defined(CAPABLE_DICTROM) && !defined(_FM77AV40EX) && !defined(_FM77AV40SX)
void META_MainWindow::do_set_use_dictcard(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | FM7_DIPSW_DICTROM_AV;
	} else {
		config.dipswitch = config.dipswitch & ~FM7_DIPSW_DICTROM_AV;
	}
}
#endif

void META_MainWindow::retranslateVolumeLabels(Ui_SoundDialog *p)
{
}

void META_MainWindow::retranslateUi(void)
{

	int z80num, jcommnum;
	z80num = -1;
	jcommnum = -1;
	Ui_MainWindowBase::retranslateUi();
	
	retranslateControlMenu("Hot Start (BREAK+RESET)", true);
	QString fdname320;
	QString fdname640;
	QString fdname1M;
	fdname320 = QApplication::translate("MenuFM7", "320K FDD", 0);
	fdname640 = QApplication::translate("MenuFM7", "2D/2DD FDD", 0);
#if defined(HAS_2HD)
	fdname1M  = QApplication::translate("MenuFM7", "2HD FDD", 0);
#elif defined(HAS_1MSFD)
	fdname1M  = QApplication::translate("MenuFM7", "8\" FDD", 0);
#else
	fdname320 = QApplication::translate("MenuFM7", "2D FDD", 0);
	fdname1M  = QApplication::translate("MenuFM7", "2HD FDD", 0);
#endif
#if defined(_FM77AV40) || defined(_FM77AV20EX) || defined(_FM77AV40EX) || defined(_FM77AV40SX)
	retranslateFloppyMenu(0, 0, fdname640);
	retranslateFloppyMenu(1, 1, fdname640);
#elif defined(_FM77AV20EX)
	retranslateFloppyMenu(0, 0, fdname640);
#else
	retranslateFloppyMenu(0, 0, fdname320);
	retranslateFloppyMenu(1, 1, fdname320);
#endif	
#if defined(HAS_2HD) || defined(HAS_1MSFD)
	retranslateFloppyMenu(2, 2, fdname1M);
	retranslateFloppyMenu(3, 3, fdname1M);
#endif	
	retranslateCMTMenu(0);
#if defined(USE_BUBBLE)
	for(int _drv = 0; _drv < USE_BUBBLE; _drv++) {
		retranslateBubbleMenu(_drv, _drv + 1);
	}		
#endif	

	{	
	}

	actionSpecial_Reset->setText(QApplication::translate("Machine", "Hot Start(BREAK+RESET)", 0));
	actionSpecial_Reset->setToolTip(QApplication::translate("Machine", "Do HOT START.\nReset with pressing BREAK key.", 0));
	
#if defined(USE_PRINTER_TYPE)
	actionPrintDevice[1]->setText(QApplication::translate("Machine", "Dempa Joystick with #1", 0));
	actionPrintDevice[1]->setToolTip(QApplication::translate("Machine", "Use joystick #1 as DEMPA's joystick.", 0));
	actionPrintDevice[2]->setText(QApplication::translate("Machine", "Dempa Joystick with #2", 0));
	actionPrintDevice[2]->setToolTip(QApplication::translate("Machine", "Use joystick #2 as DEMPA's joystick.", 0));
#endif
	
#ifdef USE_DEBUGGER
	actionDebugger[0]->setText(QApplication::translate("Machine", "Main CPU", 0));
	actionDebugger[1]->setText(QApplication::translate("Machine", "Sub  CPU", 0));
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
# ifdef WITH_Z80
	if((config.dipswitch & FM7_DIPSW_Z80CARD_ON) != 0) z80num = 2;
#  ifdef CAPABLE_JCOMMCARD
	if((config.dipswitch & FM7_DIPSW_JSUBCARD_ON) != 0) {
		if(z80num < 0) {
			jcommnum = 2;
		} else {
			jcommnum = 3;
		}
	}
#  endif
# else
#  ifdef CAPABLE_JCOMMCARD
	if((config.dipswitch & FM7_DIPSW_JSUBCARD_ON) != 0) {
		jcommnum = 2;
	}
#  endif
# endif
	
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
	if(z80num > 0) {
		actionDebugger[z80num]->setText(QApplication::translate("Machine", "Z80 CPU Board", 0));
		actionDebugger[z80num]->setVisible(true);
	}
	if(jcommnum > 0) {
		actionDebugger[jcommnum]->setText(QApplication::translate("Machine", "Japanese Communication Board", 0));
		actionDebugger[jcommnum]->setVisible(true);
	}
#endif	
	//	actionStart_Record_Movie->setText(QApplication::translate("Machine", "Start Record Movie", 0));
	//      actionStop_Record_Movie->setText(QApplication::translate("Machine", "Stop Record Movie", 0));
	// 
	// FM-7 Specified

	menuFrameSkip->setTitle(QApplication::translate("Machine", "Frame skip", 0));
	actionFrameSkip[0]->setText(QApplication::translate("Machine", "None", 0));
	actionFrameSkip[1]->setText(QApplication::translate("Machine", "1 Frame", 0));
	actionFrameSkip[2]->setText(QApplication::translate("Machine", "2 Frames", 0));
	actionFrameSkip[3]->setText(QApplication::translate("Machine", "3 Frames", 0));

	actionSyncToHsync->setText(QApplication::translate("Machine", "Sync to HSYNC", 0));
	actionSyncToHsync->setToolTip(QApplication::translate("Machine", "Emulate display syncing to HSYNC.\nExpect to emulate more accurate.", 0));

#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS) || defined(_FM8)
	menuCpuType->setTitle(QApplication::translate("Machine", "CPU Frequency (hack)", 0));
#else
	menuCpuType->setTitle(QApplication::translate("Machine", "CPU Frequency", 0));
#endif
	actionCpuType[0]->setText(QString::fromUtf8("2MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("1.2MHz"));
	
	menuBootMode->setTitle(QApplication::translate("Machine", "BOOT Mode", 0));
	menuBootMode->setToolTipsVisible(true);
	actionBootMode[0]->setText(QApplication::translate("Machine", "BASIC", 0));
	actionBootMode[0]->setToolTip(QApplication::translate("Machine", "Boot with F-BASIC.", 0));
	actionBootMode[1]->setText(QApplication::translate("Machine", "DOS", 0));	
	actionBootMode[1]->setToolTip(QApplication::translate("Machine", "DOS boot mode.\nUse for CP/M, FLEX, OS-9, R-DOS and some OSs.", 0));
	
	actionBootMode[2]->setVisible(false);
	actionBootMode[3]->setVisible(false);
#if defined(_FM77_VARIANTS)
	actionBootMode[0]->setText(QString::fromUtf8("BASIC + S1 (BANK0)"));
	actionBootMode[0]->setToolTip(QApplication::translate("Machine", "BASIC boot mode with S1 has turned ON.\nThis is FM-77 feature and I don't know about this.", 0));
	actionBootMode[0]->binds->setValue1(2);
	
	actionBootMode[4]->setText(QApplication::translate("Machine", "BASIC (BANK4)", 0));
	actionBootMode[4]->setToolTip(QApplication::translate("Machine", "Boot with F-BASIC.", 0));
	actionBootMode[4]->binds->setValue1(0);
	
	actionBootMode[5]->setText(QApplication::translate("Machine", "DOS 320K(BOOT2 / BANK5)", 0));	
	actionBootMode[5]->setToolTip(QApplication::translate("Machine", "DOS boot mode.\nUse for CP/M, FLEX, OS-9, R-DOS and some OSs.", 0));
	actionBootMode[5]->binds->setValue1(1);
	
	actionBootMode[6]->setText(QString::fromUtf8("1MB FD (BOOT1 / BANK6)"));
	actionBootMode[6]->setToolTip(QApplication::translate("Machine", "Boot from 1MB FD for FM-77.\n", 0));
	actionBootMode[6]->binds->setValue1(3);
	
	actionBootMode[1]->setText(QString::fromUtf8("BUBBLE 128K (BOOT2 + S1 / BANK1)"));
	actionBootMode[1]->setToolTip(QApplication::translate("Machine", "Boot from 128K Bubble casette.\nThis is FM-77 feature and I don't know about this.", 0));
	actionBootMode[1]->binds->setValue1(5);
	
	actionBootMode[2]->setText(QString::fromUtf8("BUBBLE 32K (BOOT1 + S1 / BANK2)"));
	actionBootMode[2]->setToolTip(QApplication::translate("Machine", "Boot from 32K Bubble casette.\nThis is FM-77 feature and I don't know about this.", 0));
	actionBootMode[2]->binds->setValue1(6);
	
	actionBootMode[3]->setText(QString::fromUtf8("RESERVE 1 (BANK3)"));
	actionBootMode[3]->setToolTip(QApplication::translate("Machine", "RESERVE AREA 1\nThis is FM-77 feature and I don't know about this.", 0));
	actionBootMode[3]->binds->setValue1(3);
	
	actionBootMode[7]->setText(QString::fromUtf8("RESERVE 2 (BANK7)"));
	actionBootMode[7]->setToolTip(QApplication::translate("Machine", "RESERVED AREA 2\nThis is FM-77 feature and I don't know about this.", 0));
	actionBootMode[7]->binds->setValue1(4);
	
	for(int i = 0; i < 7; i++) {
		if(actionBootMode[i]->binds->getValue1() == config.boot_mode) actionBootMode[i]->setChecked(true);
	}
	for(int i = 0; i < 8; i++) {
		actionBootMode[i]->setVisible(true);
		actionBootMode[i]->setEnabled(true);
	}
#elif defined(_FM8)
	actionBootMode[0]->setText(QApplication::translate("Machine", "BASIC  (SM11-14 BANK0)", 0));
	actionBootMode[0]->binds->setValue1(0);
	actionBootMode[2]->setText(QApplication::translate("Machine", "DOS    (SM11-14 BANK2)", 0));	
	actionBootMode[2]->binds->setValue1(1);
	actionBootMode[2]->setToolTip(QApplication::translate("Machine", "DOS boot mode.\nUse for CP/M, FLEX, OS-9, R-DOS and some OSs.", 0));
	
	actionBootMode[1]->setText(QApplication::translate("Machine", "BUBBLE  (SM11-14 BANK1)", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("Machine", "Boot for bubble casette.\nYou must install boot rom for this.", 0));
	actionBootMode[1]->binds->setValue1(2);
	
	actionBootMode[7]->setText(QApplication::translate("Machine", "8Inch FD (SM11-15 BANK3)", 0));
	actionBootMode[7]->setToolTip(QApplication::translate("Machine", "Boot for 8inch floppy disk.\nYou must install boot rom for this.", 0));
	actionBootMode[1]->binds->setValue1(7);
	
	actionBootMode[4]->setText(QApplication::translate("Machine", "BASIC    (SM11-15 BANK0)", 0));
	actionBootMode[4]->binds->setValue1(4);
	actionBootMode[6]->setText(QApplication::translate("Machine", "DOS      (SM11-15 BANK2)", 0));
	actionBootMode[6]->binds->setValue1(5);
	actionBootMode[4]->setToolTip(QApplication::translate("Machine", "Boot with F-BASIC.", 0));
	actionBootMode[6]->setToolTip(QApplication::translate("Machine", "DOS boot mode.\nUse for CP/M, FLEX, OS-9, R-DOS and some OSs.", 0));
	
	actionBootMode[5]->setText(QApplication::translate("Machine", "BUBBLE   (SM11-15 BANK1)", 0));
	actionBootMode[5]->setToolTip(QApplication::translate("Machine", "Boot for bubble casette.\nYou must install boot rom for this.", 0));
	actionBootMode[5]->binds->setValue1(6);
	
	actionBootMode[3]->setText(QApplication::translate("Machine", "DEBUG    (SM11-14 BANK3)", 0));
	actionBootMode[3]->setToolTip(QApplication::translate("Machine", "Boot for DEBUG.\nThis is SM11-14's feature and I don't know about this.", 0));
	actionBootMode[3]->binds->setValue1(3);
	
	for(int i = 0; i < 8; i++) {
		actionBootMode[i]->setVisible(true);
		actionBootMode[i]->setEnabled(true);
		if(actionBootMode[i]->binds->getValue1() == config.boot_mode) actionBootMode[i]->setChecked(true);
	}
#elif defined(_FM7) || defined(_FMNEW7)	
	actionBootMode[0]->setText(QApplication::translate("Machine", "BASIC (BANK0)", 0));
	actionBootMode[0]->setToolTip(QApplication::translate("Machine", "Boot with F-BASIC.", 0));
	actionBootMode[0]->binds->setValue1(0);
	
	actionBootMode[2]->setText(QApplication::translate("Machine", "DOS (BANK2)", 0));	
	actionBootMode[2]->setToolTip(QApplication::translate("Machine", "DOS boot mode.\nUse for CP/M, FLEX, OS-9, R-DOS and some OSs.", 0));
	actionBootMode[2]->binds->setValue1(1);
	
	actionBootMode[1]->setText(QApplication::translate("Machine", "Bubble Casette (BANK1)", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("Machine", "Boot for bubble casette.\nYou must install boot rom for this.", 0));
	actionBootMode[1]->binds->setValue1(2);
	
	actionBootMode[3]->setText(QApplication::translate("Machine", "Reserve (BANK3)", 0));
	actionBootMode[3]->setToolTip(QApplication::translate("Machine", "This is reserved area.\nI don't know about this.", 0));
	actionBootMode[3]->binds->setValue1(3);
	
	actionBootMode[2]->setVisible(true);
	actionBootMode[3]->setVisible(true);
	for(int i = 0; i < 4; i++) {
		if(actionBootMode[i]->binds->getValue1() == config.boot_mode) actionBootMode[i]->setChecked(true);
	}
	for(int i = 4; i < 8; i++) {
		actionBootMode[i]->setVisible(false);
		actionBootMode[i]->setEnabled(false);
	}
#else // 77AV
	actionBootMode[2]->setText(QString::fromUtf8("RESERVE"));
	actionBootMode[2]->setToolTip(QApplication::translate("Machine", "Reserved boot mode.\nThis is FM-77AV feature and I don't know about this.", 0));
	actionBootMode[3]->setVisible(false);
	
	actionBootMode[2]->setVisible(true);
	for(int i = 0; i < 2; i++) {
		if(actionBootMode[i]->binds->getValue1() == config.boot_mode) actionBootMode[i]->setChecked(true);
	}
	for(int i = 3; i < 8; i++) {
		actionBootMode[i]->setVisible(false);
		actionBootMode[i]->setEnabled(false);
	}
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
# if defined(USE_MOUSE_TYPE)
	menuMouseType->setTitle(QApplication::translate("Machine", "Mouse", 0));
	actionMouseType[0]->setText(QApplication::translate("Machine", "none", 0));
	actionMouseType[1]->setText(QApplication::translate("Machine", "JS port1", 0));
	actionMouseType[1]->setToolTip(QApplication::translate("Machine", "Connect mouse to JOYSTICK port #1.", 0));
	actionMouseType[2]->setText(QApplication::translate("Machine", "JS port2", 0));
	actionMouseType[2]->setToolTip(QApplication::translate("Machine", "Connect mouse to JOYSTICK port #2.", 0));
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
#if defined(HAS_2HD)
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
# if defined(USE_MOUSE_TYPE)
	menuMouseType->setToolTipsVisible(true);
# endif
#endif
#if defined(USE_MONITOR_TYPE) && defined(USE_GREEN_DISPLAY)
	action_GreenDisplay->setText(QApplication::translate("Machine", "Green Display (need reset)", 0));
	action_GreenDisplay->setToolTip(QApplication::translate("Machine", "Using ancient \"Green Display\" to display.\nChanges will be applied at reset, not immediately.", 0));
#endif	
#if defined(WITH_Z80)
	actionZ80CARD_ON->setText(QApplication::translate("Machine", "Connect Z80 CARD", 0));
	actionZ80CARD_ON->setToolTip(QApplication::translate("Machine", "Turn ON Z80 extra board.\nNeed to restart this emulator to change connection", 0));
	
	actionZ80_IRQ->setText(QApplication::translate("Machine", "Z80:IRQ ON", 0));
	actionZ80_IRQ->setToolTip(QApplication::translate("Machine", "Turn ON IRQ to Z80 extra board.", 0));
	
	actionZ80_FIRQ->setText(QApplication::translate("Machine", "Z80:FIRQ ON", 0));
	actionZ80_FIRQ->setToolTip(QApplication::translate("Machine", "Turn ON FIRQ to IRQ of Z80 extra card.", 0));
	
	actionZ80_NMI->setText(QApplication::translate("Machine", "Z80:NMI ON", 0));
	actionZ80_NMI->setToolTip(QApplication::translate("Machine", "Turn ON NMI to Z80 extra board.", 0));
#endif
#if defined(CAPABLE_JCOMMCARD)
	actionJCOMMCARD->setText(QApplication::translate("Machine", "Connect Japanese Communication board.", 0));
	actionJCOMMCARD->setToolTip(QApplication::translate("Machine", "Connect Japanese communication board.\nNeed to restart this emulator if you change.", 0));
#endif

	actionUART[0]->setText(QApplication::translate("Machine", "Connect RS-232C (need restart).", 0));
	actionUART[0]->setToolTip(QApplication::translate("Machine", "Connect external RS-232C board.\nNeed to restart this emulator if changed.", 0));
#if defined(_FM77AV) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
	actionUART[0]->setVisible(true);
	actionUART[0]->setEnabled(true);
#else
	actionUART[0]->setVisible(false);
	actionUART[0]->setEnabled(false);
#endif
#if defined(CAPABLE_JCOMMCARD)
	if((config.dipswitch & FM7_DIPSW_JSUBCARD_ON) != 0) {
		actionUART[0]->setText(QApplication::translate("Machine", "Turn ON RS-232C (need restart).", 0));
		actionUART[0]->setToolTip(QApplication::translate("Machine", "Turn ON RS-232C feature for Japanese communication board.\nNeed to restart this emulator if changed.", 0));
	}
#endif
#if defined(CAPABLE_DICTROM) && !defined(_FM77AV40EX) && !defined(_FM77AV40SX)
	actionDictCard->setText(QApplication::translate("Machine", "Use DICTIONARY board(need restart).", 0));
	actionDictCard->setToolTip(QApplication::translate("Machine", "Connect extra dictionary card.\nNeed to restart this emulator if changed.", 0));
#endif
	
	actionUART[1]->setText(QApplication::translate("Machine", "Connect MODEM (need restart).", 0));
	actionUART[1]->setToolTip(QApplication::translate("Machine", "Connect extra MODEM board.\nNeed to restart this emulator if changed.", 0));
	actionUART[2]->setText(QApplication::translate("Machine", "Connect MIDI (need restart).", 0));
	actionUART[2]->setToolTip(QApplication::translate("Machine", "Connect extra MIDI board.\nNeed to restart this emulator if changed.", 0));
	
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
	
	ConfigCPUTypes(2);
	ConfigCPUBootMode(USE_BOOT_MODE);
#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	actionKanjiRom = new Action_Control_7(this, using_flags);
	menuMachine->addAction(actionKanjiRom);
	actionKanjiRom->setCheckable(true);
	actionKanjiRom->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_CONNECT_KANJIROM) != 0) actionKanjiRom->setChecked(true);
	connect(actionKanjiRom, SIGNAL(toggled(bool)),
		 actionKanjiRom->fm7_binds, SLOT(do_set_kanji_rom(bool)));
#endif

#if defined(WITH_Z80)
	menuMachine->addSeparator();
	actionZ80CARD_ON = new Action_Control_7(this, using_flags);
	menuMachine->addAction(actionZ80CARD_ON);
	actionZ80CARD_ON->setCheckable(true);
	actionZ80CARD_ON->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_Z80CARD_ON) != 0) actionZ80CARD_ON->setChecked(true);
	connect(actionZ80CARD_ON, SIGNAL(toggled(bool)), actionZ80CARD_ON->fm7_binds, SLOT(do_set_z80card_on(bool)));
		
	actionZ80_IRQ = new Action_Control_7(this, using_flags);
	menuMachine->addAction(actionZ80_IRQ);
	actionZ80_IRQ->setCheckable(true);
	actionZ80_IRQ->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_Z80_IRQ_ON) != 0) actionZ80_IRQ->setChecked(true);
	connect(actionZ80_IRQ, SIGNAL(toggled(bool)), actionZ80_IRQ->fm7_binds, SLOT(do_set_z80_irq(bool)));
	connect(actionZ80_IRQ->fm7_binds, SIGNAL(sig_emu_update_config()), this, SLOT(do_emu_update_config()));

	actionZ80_FIRQ = new Action_Control_7(this, using_flags);
	menuMachine->addAction(actionZ80_FIRQ);
	actionZ80_FIRQ->setCheckable(true);
	actionZ80_FIRQ->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_Z80_FIRQ_ON) != 0) actionZ80_FIRQ->setChecked(true);
	connect(actionZ80_FIRQ, SIGNAL(toggled(bool)), actionZ80_FIRQ->fm7_binds, SLOT(do_set_z80_firq(bool)));
	connect(actionZ80_FIRQ->fm7_binds, SIGNAL(sig_emu_update_config()), this, SLOT(do_emu_update_config()));

	actionZ80_NMI = new Action_Control_7(this, using_flags);
	menuMachine->addAction(actionZ80_NMI);
	actionZ80_NMI->setCheckable(true);
	actionZ80_NMI->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_Z80_NMI_ON) != 0) actionZ80_NMI->setChecked(true);
	connect(actionZ80_NMI, SIGNAL(toggled(bool)), actionZ80_NMI->fm7_binds, SLOT(do_set_z80_nmi(bool)));
	connect(actionZ80_NMI->fm7_binds, SIGNAL(sig_emu_update_config()), this, SLOT(do_emu_update_config()));
	if((config.dipswitch & FM7_DIPSW_Z80CARD_ON) == 0) {
		actionZ80_IRQ->setVisible(false);
		actionZ80_FIRQ->setVisible(false);
		actionZ80_NMI->setVisible(false);
	}
#endif
	menuMachine->addSeparator();
	for(i = 0; i < 3; i++) {
		actionUART[i] = new Action_Control_7(this, using_flags);
		actionUART[i]->setCheckable(true);
		actionUART[i]->setVisible(true);
		actionUART[i]->fm7_binds->setValue1(i);
		menuMachine->addAction(actionUART[i]);
		connect(actionUART[i], SIGNAL(toggled(bool)), actionUART[i]->fm7_binds, SLOT(do_set_uart(bool)));
	}
	if((config.dipswitch & FM7_DIPSW_RS232C_ON) != 0) actionUART[0]->setChecked(true);
	if((config.dipswitch & FM7_DIPSW_MODEM_ON) != 0) actionUART[1]->setChecked(true);
	if((config.dipswitch & FM7_DIPSW_MIDI_ON) != 0) actionUART[2]->setChecked(true);
	
	
#if defined(CAPABLE_JCOMMCARD)
	actionJCOMMCARD = new Action_Control_7(this, using_flags);
	menuMachine->addAction(actionJCOMMCARD);
	actionJCOMMCARD->setCheckable(true);
	actionJCOMMCARD->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_JSUBCARD_ON) != 0) actionJCOMMCARD->setChecked(true);
	connect(actionJCOMMCARD, SIGNAL(toggled(bool)), actionJCOMMCARD->fm7_binds, SLOT(do_set_jcommcard(bool)));
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
	actionSyncToHsync = new Action_Control_7(this, using_flags);	
	menuMachine->addAction(actionSyncToHsync);
	actionSyncToHsync->setCheckable(true);
	actionSyncToHsync->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_SYNC_TO_HSYNC) != 0) actionSyncToHsync->setChecked(true);
	connect(actionSyncToHsync, SIGNAL(toggled(bool)),
			actionSyncToHsync->fm7_binds, SLOT(do_set_hsync(bool)));

#if defined(CAPABLE_DICTROM) && !defined(_FM77AV40EX) && !defined(_FM77AV40SX)
	actionDictCard = new Action_Control_7(this, using_flags);
	menuMachine->addAction(actionDictCard);
	actionDictCard->setCheckable(true);
	actionDictCard->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_DICTROM_AV) != 0) actionDictCard->setChecked(true);
	connect(actionDictCard, SIGNAL(toggled(bool)), this, SLOT(do_set_use_dictcard(bool)));
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
# if defined(HAS_2HD)
	action_1MFloppy = new Action_Control_7(this, using_flags);	
	menuMachine->addAction(action_1MFloppy);
	action_1MFloppy->setCheckable(true);
	action_1MFloppy->setVisible(true);
	if((config.dipswitch & FM7_DIPSW_CONNECT_1MFDC) != 0) action_1MFloppy->setChecked(true);
	connect(action_1MFloppy, SIGNAL(toggled(bool)),
			action_1MFloppy->fm7_binds, SLOT(do_set_1MFloppy(bool)));
# endif
# if defined(USE_GREEN_DISPLAY) && defined(USE_MONITOR_TYPE)
	action_GreenDisplay = new Action_Control_7(this, using_flags);	
	menuMachine->addAction(action_GreenDisplay);
	action_GreenDisplay->setCheckable(true);
	action_GreenDisplay->setVisible(true);
	if(config.monitor_type == FM7_MONITOR_GREEN) action_GreenDisplay->setChecked(true);
	connect(action_GreenDisplay, SIGNAL(toggled(bool)), action_GreenDisplay->fm7_binds, SLOT(do_set_green_display(bool)));
	connect(action_GreenDisplay->fm7_binds, SIGNAL(sig_emu_update_config()), this, SLOT(do_emu_update_config()));
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



