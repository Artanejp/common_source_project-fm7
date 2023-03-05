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

void META_MainWindow::do_set_autokey_5_8(void)
{
	if(p_config == nullptr) return;
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	int val = cp->data().value<int>();

	switch(val) {
	case 0: // Not Auto Key:
		p_config->dipswitch = p_config->dipswitch & ~(FM7_DIPSW_SELECT_5_OR_8KEY | FM7_DIPSW_AUTO_5_OR_8KEY);
		break;
	case 1: // Auto 5
		p_config->dipswitch = p_config->dipswitch | (FM7_DIPSW_SELECT_5_OR_8KEY | FM7_DIPSW_AUTO_5_OR_8KEY);
		break;
	case 2: // Auto 8
		p_config->dipswitch = (p_config->dipswitch | FM7_DIPSW_AUTO_5_OR_8KEY) & ~FM7_DIPSW_SELECT_5_OR_8KEY;
		break;
	default:// Not Auto Key:
		p_config->dipswitch = p_config->dipswitch & ~(FM7_DIPSW_SELECT_5_OR_8KEY | FM7_DIPSW_AUTO_5_OR_8KEY);
		break;
	}
}

void META_MainWindow::do_set_frameskip()
{
	QAction *cp = qobject_cast<QAction*>(QObject::sender());
	if(cp == nullptr) return;
	uint32_t nval = cp->data().value<uint32_t>();

	if(p_config == nullptr) return;
	p_config->dipswitch &= 0xcfffffff;
	p_config->dipswitch |= (nval & 0x30000000);
}

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

	actionSpecial_Reset[0]->setText(QApplication::translate("Machine", "Hot Start(BREAK+RESET)", 0));
	actionSpecial_Reset[0]->setToolTip(QApplication::translate("Machine", "Do HOT START.\nReset with pressing BREAK key.", 0));

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
	if((p_config->dipswitch & FM7_DIPSW_Z80CARD_ON) != 0) z80num = 2;
#  ifdef CAPABLE_JCOMMCARD
	if((p_config->dipswitch & FM7_DIPSW_JSUBCARD_ON) != 0) {
		if(z80num < 0) {
			jcommnum = 2;
		} else {
			jcommnum = 3;
		}
	}
#  endif
# else
#  ifdef CAPABLE_JCOMMCARD
	if((p_config->dipswitch & FM7_DIPSW_JSUBCARD_ON) != 0) {
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

	menuFrameSkip->setTitle(QApplication::translate("MachineFM7", "Frame skip", 0));
	actionFrameSkip[0]->setText(QApplication::translate("MachineFM7", "None", 0));
	actionFrameSkip[1]->setText(QApplication::translate("MachineFM7", "1 Frame", 0));
	actionFrameSkip[2]->setText(QApplication::translate("MachineFM7", "2 Frames", 0));
	actionFrameSkip[3]->setText(QApplication::translate("MachineFM7", "3 Frames", 0));

	actionSyncToHsync->setText(QApplication::translate("MachineFM7", "Sync to HSYNC", 0));
	actionSyncToHsync->setToolTip(QApplication::translate("MachineFM7", "Emulate display syncing to HSYNC.\nExpect to emulate more accurate.", 0));

# ifdef WITH_Z80
	retranslateOpMenuZ80(true);
	action_DriveInOpCode->setText(QApplication::translate("MachineFM7", "Z80:Drive VM in M1/R/W Cycle", 0));
	action_DriveInOpCode->setToolTip(QApplication::translate("MachineFM7", "Process some events and wait per instruction.\nMaybe emulation more correctness for Z80 card if available.", 0));

#if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS) || defined(_FM8)
	menuCpuType->setTitle(QApplication::translate("MachineFM7", "CPU Frequency (hack)", 0));
#else
	menuCpuType->setTitle(QApplication::translate("MachineFM7", "CPU Frequency", 0));
#endif
	actionCpuType[0]->setText(QString::fromUtf8("2MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("1.2MHz"));

	menuBootMode->setTitle(QApplication::translate("MachineFM7", "BOOT Mode", 0));
	menuBootMode->setToolTipsVisible(true);
	actionBootMode[0]->setText(QApplication::translate("MachineFM7", "BASIC", 0));
	actionBootMode[0]->setToolTip(QApplication::translate("MachineFM7", "Boot with F-BASIC.", 0));
	actionBootMode[1]->setText(QApplication::translate("MachineFM7", "DOS", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("MachineFM7", "DOS boot mode.\nUse for CP/M, FLEX, OS-9, R-DOS and some OSs.", 0));

	actionBootMode[2]->setVisible(false);
	actionBootMode[3]->setVisible(false);
#if defined(_FM77_VARIANTS)
	actionBootMode[0]->setText(QString::fromUtf8("BASIC + S1 (BANK0)"));
	actionBootMode[0]->setToolTip(QApplication::translate("MachineFM7", "BASIC boot mode with S1 has turned ON.\nThis is FM-77 feature and I don't know about this.", 0));
	actionBootMode[0]->setData(QVariant((int)2));

	actionBootMode[4]->setText(QApplication::translate("MachineFM7", "BASIC (BANK4)", 0));
	actionBootMode[4]->setToolTip(QApplication::translate("MachineFM7", "Boot with F-BASIC.", 0));
	actionBootMode[4]->setData(QVariant((int)0));

	actionBootMode[5]->setText(QApplication::translate("MachineFM7", "DOS 320K(BOOT2 / BANK5)", 0));
	actionBootMode[5]->setToolTip(QApplication::translate("MachineFM7", "DOS boot mode.\nUse for CP/M, FLEX, OS-9, R-DOS and some OSs.", 0));
	actionBootMode[5]->setData(QVariant((int)1));

	actionBootMode[6]->setText(QString::fromUtf8("1MB FD (BOOT1 / BANK6)"));
	actionBootMode[6]->setToolTip(QApplication::translate("MachineFM7", "Boot from 1MB FD for FM-77.\n", 0));
	actionBootMode[6]->setData(QVariant((int)3));

	actionBootMode[1]->setText(QString::fromUtf8("BUBBLE 128K (BOOT2 + S1 / BANK1)"));
	actionBootMode[1]->setToolTip(QApplication::translate("MachineFM7", "Boot from 128K Bubble casette.\nThis is FM-77 feature and I don't know about this.", 0));
	actionBootMode[1]->setData(QVariant((int)5));

	actionBootMode[2]->setText(QString::fromUtf8("BUBBLE 32K (BOOT1 + S1 / BANK2)"));
	actionBootMode[2]->setToolTip(QApplication::translate("MachineFM7", "Boot from 32K Bubble casette.\nThis is FM-77 feature and I don't know about this.", 0));
	actionBootMode[2]->setData(QVariant((int)6));

	actionBootMode[3]->setText(QString::fromUtf8("RESERVE 1 (BANK3)"));
	actionBootMode[3]->setToolTip(QApplication::translate("MachineFM7", "RESERVE AREA 1\nThis is FM-77 feature and I don't know about this.", 0));
	actionBootMode[3]->setData(QVariant((int)3));

	actionBootMode[7]->setText(QString::fromUtf8("RESERVE 2 (BANK7)"));
	actionBootMode[7]->setToolTip(QApplication::translate("MachineFM7", "RESERVED AREA 2\nThis is FM-77 feature and I don't know about this.", 0));
	actionBootMode[7]->setData(QVariant((int)4));

	for(int i = 0; i < 7; i++) {
		int _n = actionBootMode[i]->data().value<int>();
		if(_n == p_config->boot_mode) actionBootMode[i]->setChecked(true);
	}
	for(int i = 0; i < 8; i++) {
		actionBootMode[i]->setVisible(true);
		actionBootMode[i]->setEnabled(true);
	}
#elif defined(_FM8)
	actionBootMode[0]->setText(QApplication::translate("MachineFM7", "BASIC  (SM11-14 BANK0)", 0));
	actionBootMode[0]->setData(QVariant((int)0));

	actionBootMode[2]->setText(QApplication::translate("MachineFM7", "DOS    (SM11-14 BANK2)", 0));
	actionBootMode[2]->setData(QVariant((int)1));

	actionBootMode[2]->setToolTip(QApplication::translate("MachineFM7", "DOS boot mode.\nUse for CP/M, FLEX, OS-9, R-DOS and some OSs.", 0));

	actionBootMode[1]->setText(QApplication::translate("MachineFM7", "BUBBLE  (SM11-14 BANK1)", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("MachineFM7", "Boot for bubble casette.\nYou must install boot rom for this.", 0));
	actionBootMode[1]->setData(QVariant((int)2));

	actionBootMode[7]->setText(QApplication::translate("MachineFM7", "8Inch FD (SM11-15 BANK3)", 0));
	actionBootMode[7]->setToolTip(QApplication::translate("MachineFM7", "Boot for 8inch floppy disk.\nYou must install boot rom for this.", 0));
	actionBootMode[7]->setData(QVariant((int)7));

	actionBootMode[4]->setText(QApplication::translate("MachineFM7", "BASIC    (SM11-15 BANK0)", 0));
	actionBootMode[4]->setData(QVariant((int)4));

	actionBootMode[6]->setText(QApplication::translate("MachineFM7", "DOS      (SM11-15 BANK2)", 0));
	actionBootMode[6]->setData(QVariant((int)5));

	actionBootMode[4]->setToolTip(QApplication::translate("MachineFM7", "Boot with F-BASIC.", 0));
	actionBootMode[6]->setToolTip(QApplication::translate("MachineFM7", "DOS boot mode.\nUse for CP/M, FLEX, OS-9, R-DOS and some OSs.", 0));

	actionBootMode[5]->setText(QApplication::translate("MachineFM7", "BUBBLE   (SM11-15 BANK1)", 0));
	actionBootMode[5]->setToolTip(QApplication::translate("MachineFM7", "Boot for bubble casette.\nYou must install boot rom for this.", 0));
	actionBootMode[5]->setData(QVariant((int)6));


	actionBootMode[3]->setText(QApplication::translate("MachineFM7", "DEBUG    (SM11-14 BANK3)", 0));
	actionBootMode[3]->setToolTip(QApplication::translate("MachineFM7", "Boot for DEBUG.\nThis is SM11-14's feature and I don't know about this.", 0));
	actionBootMode[3]->setData(QVariant((int)3));

	for(int i = 0; i < 8; i++) {
		actionBootMode[i]->setVisible(true);
		actionBootMode[i]->setEnabled(true);
		int _n = actionBootMode[i]->data().value<int>();
		if(_n == p_config->boot_mode) actionBootMode[i]->setChecked(true);
	}
#elif defined(_FM7) || defined(_FMNEW7)
	actionBootMode[0]->setText(QApplication::translate("MachineFM7", "BASIC (BANK0)", 0));
	actionBootMode[0]->setToolTip(QApplication::translate("MachineFM7", "Boot with F-BASIC.", 0));
	actionBootMode[0]->setData(QVariant((int)0));

	actionBootMode[2]->setText(QApplication::translate("MachineFM7", "DOS (BANK2)", 0));
	actionBootMode[2]->setToolTip(QApplication::translate("MachineFM7", "DOS boot mode.\nUse for CP/M, FLEX, OS-9, R-DOS and some OSs.", 0));
	actionBootMode[2]->setData(QVariant((int)1));

	actionBootMode[1]->setText(QApplication::translate("MachineFM7", "Bubble Casette (BANK1)", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("MachineFM7", "Boot for bubble casette.\nYou must install boot rom for this.", 0));
	actionBootMode[1]->setData(QVariant((int)2));

	actionBootMode[3]->setText(QApplication::translate("MachineFM7", "Reserve (BANK3)", 0));
	actionBootMode[3]->setToolTip(QApplication::translate("MachineFM7", "This is reserved area.\nI don't know about this.", 0));
	actionBootMode[3]->setData(QVariant((int)3));

	actionBootMode[2]->setVisible(true);
	actionBootMode[3]->setVisible(true);
	for(int i = 0; i < 4; i++) {
		int _n = actionBootMode[i]->data().value<int>();
		if(_n == p_config->boot_mode) actionBootMode[i]->setChecked(true);
	}
	for(int i = 4; i < 8; i++) {
		actionBootMode[i]->setVisible(false);
		actionBootMode[i]->setEnabled(false);
	}
#else // 77AV
	actionBootMode[2]->setText(QString::fromUtf8("RESERVE"));
	actionBootMode[2]->setToolTip(QApplication::translate("MachineFM7", "Reserved boot mode.\nThis is FM-77AV feature and I don't know about this.", 0));
	actionBootMode[3]->setVisible(false);

	actionBootMode[2]->setVisible(true);
	for(int i = 0; i < 2; i++) {
		int _n = actionBootMode[i]->data().value<int>();
		if(_n == p_config->boot_mode) actionBootMode[i]->setChecked(true);
	}
	for(int i = 3; i < 8; i++) {
		actionBootMode[i]->setVisible(false);
		actionBootMode[i]->setEnabled(false);
	}
#endif

#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	actionKanjiRom->setText(QApplication::translate("MachineFM7", "Kanji ROM(Need restart)", 0));
	actionKanjiRom->setToolTip(QApplication::translate("MachineFM7", "Connect KANJI ROM.Need restart emulator if changed.", 0));
#endif
#if defined(_FM8)
	actionRamProtect->setText(QApplication::translate("MachineFM7", "BANK PROTECT($FD0F/hack)", 0));
	actionRamProtect->setToolTip(QApplication::translate("MachineFM7", "Protect bank setting if checked.\nUnchecked to simulate FM-7.\nUseful for some software tranferring to URA-RAM.", 0));
#elif defined(_FM7) || defined(_FMNEW7)
	actionCycleSteal->setText(QApplication::translate("MachineFM7", "Cycle Steal(hack)", 0));
	actionCycleSteal->setToolTip(QApplication::translate("MachineFM7", "Enabling cycle steal to be faster drawing.\nThis is hack for FM-7.", 0));
#else
	actionCycleSteal->setText(QApplication::translate("MachineFM7", "Cycle Steal", 0));
	actionCycleSteal->setToolTip(QApplication::translate("MachineFM7", "Enabling cycle steal to be faster drawing.", 0));
#endif
	menuSoundDevice->setTitle(QApplication::translate("MachineFM7", "Sound Boards", 0));
#if defined(USE_SOUND_TYPE)
# if defined(_FM8)
	actionSoundDevice[0]->setVisible(true);
	actionSoundDevice[1]->setVisible(true);
	actionSoundDevice[0]->setText(QApplication::translate("MachineFM7", "Beep Only", 0));
	actionSoundDevice[0]->setToolTip(QApplication::translate("MachineFM7", "Use only buzzer as sound device.", 0));
	actionSoundDevice[1]->setText(QApplication::translate("MachineFM7", "PSG (hack)", 0));
	actionSoundDevice[1]->setToolTip(QApplication::translate("MachineFM7", "Set FM-7 compatible PSG.\nThis emulates extra PSG board made by third party.", 0));
# elif defined(_FM77AV_VARIANTS)
	actionSoundDevice[0]->setVisible(false);
	actionSoundDevice[2]->setVisible(false);
	actionSoundDevice[4]->setVisible(false);
	actionSoundDevice[6]->setVisible(false);
	actionSoundDevice[1]->setText(QApplication::translate("MachineFM7", "OPN", 0));
	actionSoundDevice[3]->setText(QApplication::translate("MachineFM7", "OPN+WHG", 0));
	actionSoundDevice[5]->setText(QApplication::translate("MachineFM7", "OPN+THG", 0));
	actionSoundDevice[7]->setText(QApplication::translate("MachineFM7", "OPN+WHG+THG", 0));

	actionSoundDevice[1]->setToolTip(QApplication::translate("MachineFM7", "Using only default FM synthesizer board.", 0));
	actionSoundDevice[3]->setToolTip(QApplication::translate("MachineFM7", "Using default FM synthesizer board\nand WHG second FM synthesizer board.", 0));
	actionSoundDevice[5]->setToolTip(QApplication::translate("MachineFM7", "Using default FM synthesizer board\nand THG third FM synthesizer board.", 0));
	actionSoundDevice[7]->setToolTip(QApplication::translate("MachineFM7", "Using default FM synthesizer board\nand WHG second FM synthesizer board\nand THG third FM synthesizer board.", 0));
# else
	actionSoundDevice[0]->setText(QApplication::translate("MachineFM7", "PSG", 0));
	actionSoundDevice[1]->setText(QApplication::translate("MachineFM7", "PSG+OPN", 0));
	actionSoundDevice[2]->setText(QApplication::translate("MachineFM7", "PSG+WHG", 0));
	actionSoundDevice[3]->setText(QApplication::translate("MachineFM7", "PSG+OPN+WHG", 0));
	actionSoundDevice[4]->setText(QApplication::translate("MachineFM7", "PSG+THG", 0));
	actionSoundDevice[5]->setText(QApplication::translate("MachineFM7", "PSG+OPN+THG", 0));
	actionSoundDevice[6]->setText(QApplication::translate("MachineFM7", "PSG+WHG+THG", 0));
	actionSoundDevice[7]->setText(QApplication::translate("MachineFM7", "PSG+OPN+WHG+THG", 0));

	actionSoundDevice[0]->setToolTip(QApplication::translate("MachineFM7", "Using only default PSG.", 0));
	actionSoundDevice[1]->setToolTip(QApplication::translate("MachineFM7", "Using default PSG \nand Fujitsu's FM synthesizer board.", 0));
	actionSoundDevice[2]->setToolTip(QApplication::translate("MachineFM7", "Using default PSG \nand WHG second FM synthesizer board.", 0));
	actionSoundDevice[3]->setToolTip(QApplication::translate("MachineFM7", "Using default PSG \nand Fujitsu's FM synthesizer board\nand WHG second FM synthesizer board.", 0));
	actionSoundDevice[4]->setToolTip(QApplication::translate("MachineFM7", "Using default PSG \nand THG third FM synthesizer board.", 0));
	actionSoundDevice[5]->setToolTip(QApplication::translate("MachineFM7", "Using default PSG \nand Fujitsu's FM synthesizer board\nand THG third FM synthesizer board.", 0));
	actionSoundDevice[6]->setToolTip(QApplication::translate("MachineFM7", "Using default PSG \nand WHG 2nd FM sythesizer board\nand THG third FM synthesizer board.", 0));
	actionSoundDevice[7]->setToolTip(QApplication::translate("MachineFM7", "Using default PSG \nand Fujitsu's FM synthesizer board\nand WHG second FM synthesizer board\nand THG third FM synthesizer board.", 0));
# endif
#endif
#if !defined(_FM8)
# if defined(USE_MOUSE_TYPE)
	menuMouseType->setTitle(QApplication::translate("MachineFM7", "Mouse", 0));
	actionMouseType[0]->setText(QApplication::translate("MachineFM7", "none", 0));
	actionMouseType[1]->setText(QApplication::translate("MachineFM7", "JS port1", 0));
	actionMouseType[1]->setToolTip(QApplication::translate("MachineFM7", "Connect mouse to JOYSTICK port #1.", 0));
	actionMouseType[2]->setText(QApplication::translate("MachineFM7", "JS port2", 0));
	actionMouseType[2]->setToolTip(QApplication::translate("MachineFM7", "Connect mouse to JOYSTICK port #2.", 0));
# endif

# if defined(_FM77AV_VARIANTS) || defined(_FM77_VARIANTS)
	actionExtRam->setText(QApplication::translate("MachineFM7", "Use Extra RAM (Need reboot)", 0));
	actionExtRam->setToolTip(QApplication::translate("MachineFM7", "Using extra ram block.\nNeed to reboot if changed.", 0));
# endif
#endif
#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	action_320kFloppy->setText(QApplication::translate("MachineFM7", "Connect 320KB FDD(Need Restart)", 0));
	action_320kFloppy->setToolTip(QApplication::translate("MachineFM7", "Connect 2D floppy drive.\nNeed to restart emulator if changed.", 0));
#endif
#if defined(HAS_2HD)
	action_1MFloppy->setText(QApplication::translate("MachineFM7", "Connect 1MB FDD(Need Restart)", 0));
	action_1MFloppy->setToolTip(QApplication::translate("MachineFM7", "**Note: This option still not implemented**\nConnect 2HD (or 8inch) floppy drive.\nNeed to restart emulator if changed.\n", 0));
#endif
	menuAuto5_8Key->setTitle(QApplication::translate("MachineFM7", "Auto Stop Ten Key (hack)", 0));
	action_Neither_5_or_8key->setText(QApplication::translate("MachineFM7", "None used.", 0));
	action_Auto_5key->setText(QApplication::translate("MachineFM7", "Use 5", 0));
	action_Auto_5key->setToolTip(QApplication::translate("MachineFM7", "Use '5' key to stop with some GAMES.\nUseful for games using '2 4 6 8' to move character.", 0));
	action_Auto_8key->setText(QApplication::translate("MachineFM7", "Use 8", 0));
	action_Auto_8key->setToolTip(QApplication::translate("MachineFM7", "Use '8' key to stop with some GAMES.\nUseful for games using '1 2 3 5' to move character.", 0));
	// Set Labels
	menuMachine->setToolTipsVisible(true);
	menuAuto5_8Key->setToolTipsVisible(true);

#if !defined(_FM8)
# if defined(USE_MOUSE_TYPE)
	menuMouseType->setToolTipsVisible(true);
# endif
#endif

#if defined(USE_MONITOR_TYPE) && defined(USE_GREEN_DISPLAY)
	actionMonitorType[0]->setText(QApplication::translate("MachineFM7", "Color Display (need reset)", 0));
	actionMonitorType[0]->setToolTip(QApplication::translate("MachineFM7", "Using color display.\nChanges will be applied at reset, not immediately.", 0));
	actionMonitorType[1]->setText(QApplication::translate("MachineFM7", "Green Display (need reset)", 0));
	actionMonitorType[1]->setToolTip(QApplication::translate("MachineFM7", "Using ancient \"Green Display\" to display.\nChanges will be applied at reset, not immediately.", 0));
#endif
#if defined(WITH_Z80)
	actionZ80CARD_ON->setText(QApplication::translate("MachineFM7", "Connect Z80 CARD", 0));
	actionZ80CARD_ON->setToolTip(QApplication::translate("MachineFM7", "Turn ON Z80 extra board.\nNeed to restart this emulator to change connection", 0));

	actionZ80_IRQ->setText(QApplication::translate("MachineFM7", "Z80:IRQ ON", 0));
	actionZ80_IRQ->setToolTip(QApplication::translate("MachineFM7", "Turn ON IRQ to Z80 extra board.", 0));

	actionZ80_FIRQ->setText(QApplication::translate("MachineFM7", "Z80:FIRQ ON", 0));
	actionZ80_FIRQ->setToolTip(QApplication::translate("MachineFM7", "Turn ON FIRQ to IRQ of Z80 extra card.", 0));

	actionZ80_NMI->setText(QApplication::translate("MachineFM7", "Z80:NMI ON", 0));
	actionZ80_NMI->setToolTip(QApplication::translate("MachineFM7", "Turn ON NMI to Z80 extra board.", 0));
#endif
#if defined(CAPABLE_JCOMMCARD)
	actionJCOMMCARD->setText(QApplication::translate("MachineFM7", "Connect Japanese Communication board.", 0));
	actionJCOMMCARD->setToolTip(QApplication::translate("MachineFM7", "Connect Japanese communication board.\nNeed to restart this emulator if you change.", 0));
#endif

	actionUART[0]->setText(QApplication::translate("MachineFM7", "Connect RS-232C (need restart).", 0));
	actionUART[0]->setToolTip(QApplication::translate("MachineFM7", "Connect external RS-232C board.\nNeed to restart this emulator if changed.", 0));
#if defined(_FM77AV) || defined(_FM7) || defined(_FMNEW7) || defined(_FM77_VARIANTS)
	actionUART[0]->setVisible(true);
	actionUART[0]->setEnabled(true);
#else
	actionUART[0]->setVisible(false);
	actionUART[0]->setEnabled(false);
#endif
#if defined(CAPABLE_JCOMMCARD)
	if((p_config->dipswitch & FM7_DIPSW_JSUBCARD_ON) != 0) {
		actionUART[0]->setText(QApplication::translate("MachineFM7", "Turn ON RS-232C (need restart).", 0));
		actionUART[0]->setToolTip(QApplication::translate("MachineFM7", "Turn ON RS-232C feature for Japanese communication board.\nNeed to restart this emulator if changed.", 0));
	}
#endif
#if defined(CAPABLE_DICTROM) && !defined(_FM77AV40EX) && !defined(_FM77AV40SX)
	actionDictCard->setText(QApplication::translate("MachineFM7", "Use DICTIONARY board(need restart).", 0));
	actionDictCard->setToolTip(QApplication::translate("MachineFM7", "Connect extra dictionary card.\nNeed to restart this emulator if changed.", 0));
#endif

	actionUART[1]->setText(QApplication::translate("MachineFM7", "Connect MODEM (need restart).", 0));
	actionUART[1]->setToolTip(QApplication::translate("MachineFM7", "Connect extra MODEM board.\nNeed to restart this emulator if changed.", 0));
	actionUART[2]->setText(QApplication::translate("MachineFM7", "Connect MIDI (need restart).", 0));
	actionUART[2]->setToolTip(QApplication::translate("MachineFM7", "Connect extra MIDI board.\nNeed to restart this emulator if changed.", 0));

	menuCpuType->setToolTipsVisible(true);
	menuBootMode->setToolTipsVisible(true);
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
	uint32_t i;
	uint32_t _val;

	menuFrameSkip = new QMenu(menuMachine);
	menuFrameSkip->setObjectName(QString::fromUtf8("menuControl_FrameSkip"));
	actionGroup_FrameSkip = new QActionGroup(this);
	actionGroup_FrameSkip->setExclusive(true);

	for(i = 0; i < 4; i++) {
		uint32_t __bit = ((uint32_t)i) << 28;
		const uint32_t 	__mask = 3 << 28;
		const uint32_t __skip =  p_config->dipswitch & __mask;
		SET_ACTION_DIPSWITCH_CONNECT(actionFrameSkip[i], __bit, __mask, __skip,
									 SIGNAL(triggered()),  SLOT(do_set_frameskip()));
		actionGroup_FrameSkip->addAction(actionFrameSkip[i]);
		menuFrameSkip->addAction(actionFrameSkip[i]);
	}
	menuMachine->addAction(menuFrameSkip->menuAction());

	ConfigCPUTypes(2);
	ConfigCPUBootMode(USE_BOOT_MODE);
#if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionKanjiRom, FM7_DIPSW_CONNECT_KANJIROM,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionKanjiRom);
#endif

#if defined(WITH_Z80)
	menuMachine->addSeparator();
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionZ80CARD_ON, FM7_DIPSW_Z80CARD_ON,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionZ80CARD_ON);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionZ80_IRQ, FM7_DIPSW_Z80_IRQ_ON,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionZ80_IRQ);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionZ80_FIRQ, FM7_DIPSW_Z80_FIRQ_ON,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionZ80_FIRQ);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionZ80_NMI, FM7_DIPSW_Z80_NMI_ON,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionZ80_NMI);

	if((p_config->dipswitch & FM7_DIPSW_Z80CARD_ON) == 0) {
		actionZ80_IRQ->setVisible(false);
		actionZ80_FIRQ->setVisible(false);
		actionZ80_NMI->setVisible(false);
	}
#endif
	menuMachine->addSeparator();

	for(i = 0; i < 3; i++) {
		const uint32_t uart_list[] = {
			FM7_DIPSW_RS232C_ON,
			FM7_DIPSW_MODEM_ON,
			FM7_DIPSW_MIDI_ON,
		};

		SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionUART[i],
									 uart_list[i],
									 p_config->dipswitch,
									 SIGNAL(toggled(bool)),
									 SLOT(do_set_single_dipswitch(bool)));
	}

#if defined(CAPABLE_JCOMMCARD)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionJCOMMCARD, FM7_DIPSW_JSUBCARD_ON,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionJCOMMCARD);
#endif

#if defined(_FM8)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionRamProtect, FM7_DIPSW_FM8_PROTECT_FD0F,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionRamProtect);

#else
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionCycleSteal, FM7_DIPSW_CYCLESTEAL,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionCycleSteal);
#endif
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionSyncToHsync, FM7_DIPSW_SYNC_TO_HSYNC,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionSyncToHsync);


#if defined(CAPABLE_DICTROM) && !defined(_FM77AV40EX) && !defined(_FM77AV40SX)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionDictCard, FM7_DIPSW_DICTROM_AV,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionDictCard);
#endif
#if defined(_FM77_VARIANTS) || defined(_FM77AV_VARIANTS)
# if defined(_FM77AV40) || defined(_FM77AV40SX) || defined(_FM77AV40EX) || defined(_FM77_VARIANTS)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionExtRam, FM7_DIPSW_EXTRAM,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
# elif defined(_FM77AV_VARIANTS)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionExtRam, FM7_DIPSW_EXTRAM_AV,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
# endif
	menuMachine->addAction(actionExtRam);

#endif

# if defined(_FM8) || defined(_FM7) || defined(_FMNEW7)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(action_320kFloppy, FM7_DIPSW_CONNECT_320KFDC,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(action_320kFloppy);
# endif
# if defined(HAS_2HD)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(action_1MFloppy, FM7_DIPSW_CONNECT_1MFDC,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(action_1MFloppy);
# endif
	uint32_t tmpv = p_config->dipswitch & (FM7_DIPSW_SELECT_5_OR_8KEY | FM7_DIPSW_AUTO_5_OR_8KEY);

	menuAuto5_8Key = new QMenu(menuMachine);
	menuAuto5_8Key->setObjectName(QString::fromUtf8("menuControl_Auto5_8Key"));
	menuMachine->addAction(menuAuto5_8Key->menuAction());

	actionGroup_Auto_5_8key = new QActionGroup(this);
	actionGroup_Auto_5_8key->setExclusive(true);

	action_Neither_5_or_8key = new Action_Control(this, using_flags);
	action_Neither_5_or_8key->setCheckable(true);
	action_Neither_5_or_8key->setVisible(true);
	action_Neither_5_or_8key->setData(QVariant((int)0));
	actionGroup_Auto_5_8key->addAction(action_Neither_5_or_8key);
	menuAuto5_8Key->addAction(action_Neither_5_or_8key);
	if((tmpv &  FM7_DIPSW_AUTO_5_OR_8KEY) == 0) action_Neither_5_or_8key->setChecked(true);
	connect(action_Neither_5_or_8key, SIGNAL(triggered()), this, SLOT(do_set_autokey_5_8()));

	action_Auto_5key = new Action_Control(this, using_flags);
	action_Auto_5key->setCheckable(true);
	action_Auto_5key->setVisible(true);
	action_Auto_5key->setData(QVariant((int)1));
	actionGroup_Auto_5_8key->addAction(action_Auto_5key);
	menuAuto5_8Key->addAction(action_Auto_5key);
	if((tmpv &  FM7_DIPSW_AUTO_5_OR_8KEY) != 0) {
		if((tmpv &  FM7_DIPSW_SELECT_5_OR_8KEY) != 0){
			action_Auto_5key->setChecked(true);
		}
	}
	connect(action_Auto_5key, SIGNAL(triggered()), this, SLOT(do_set_autokey_5_8()));

	action_Auto_8key = new Action_Control(this, using_flags);
	action_Auto_8key->setCheckable(true);
	action_Auto_8key->setVisible(true);
	action_Auto_8key->setData(QVariant((int)2));
	actionGroup_Auto_5_8key->addAction(action_Auto_8key);
	menuAuto5_8Key->addAction(action_Auto_8key);
	if((tmpv &  FM7_DIPSW_AUTO_5_OR_8KEY) != 0) {
		if((tmpv &  FM7_DIPSW_SELECT_5_OR_8KEY) == 0){
			action_Auto_8key->setChecked(true);
		}
	}
	connect(action_Auto_8key, SIGNAL(triggered()), this, SLOT(do_set_autokey_5_8()));
}


META_MainWindow::META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE
