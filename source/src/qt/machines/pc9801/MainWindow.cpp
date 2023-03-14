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
#if defined(HAS_SUB_V30)
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
# elif  defined(_PC9801VX)
	actionCpuType[0]->setText(QString::fromUtf8("80286 10MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("80286 8MHz"));
	actionCpuType[2]->setText(QString::fromUtf8("V30 10MHz"));
	actionCpuType[3]->setText(QString::fromUtf8("V30 8MHz"));
# elif  defined(_PC98XL)
	actionCpuType[0]->setText(QString::fromUtf8("80286 10MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("80286 8MHz"));
# elif  defined(_PC9801RA) || defined(_PC98RL)
	// ToDo: PC98RL's display rotate.
	#if defined(HAS_SUB_V30)
	actionCpuType[0]->setText(QString::fromUtf8("80386 20MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("80386 16MHz"));
	actionCpuType[2]->setText(QString::fromUtf8("V30 10MHz"));
	actionCpuType[3]->setText(QString::fromUtf8("V30 8MHz"));
	// ToDo: More smart solution. 20230315 K.O
	if((((0x1) << DIPSWITCH_POSITION_USE_V30) & p_config->dipswitch) == 0) {
		actionCpuType[2]->setEnabled(false);
		actionCpuType[3]->setEnabled(false);
	}
	#else
	actionCpuType[0]->setText(QString::fromUtf8("80386 20MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("80386 16MHz"));
	#endif
# elif  defined(_PC98XA)
	actionCpuType[0]->setText(QString::fromUtf8("80286 8MHz"));
	actionCpuType[1]->setVisible(false);
# endif
#endif

//	actionRAM_512K->setText(QApplication::translate("MainWindow", "512KB RAM", 0));
//	actionRAM_512K->setToolTip(QApplication::translate("MainWindow", "Set lower RAM size to 512KB(not 640KB).\nMaybe for backward compatibility.", 0));

	actionINIT_MEMSW->setText(QApplication::translate("MainWindow", "INIT MEMSW(need RESET)", 0));
	actionINIT_MEMSW->setToolTip(QApplication::translate("MainWindow", "Initialize memory switch.\nEffects after resetting.", 0));


	actionGDC_FAST->setText(QApplication::translate("MainWindow", "FAST GDC", 0));
	actionGDC_FAST->setToolTip(QApplication::translate("MainWindow", "Set GDC clock to 5MHz when checked.\nSet to 2.5MHz when not checked.", 0));
	actionFDD_INTFDD_OFF->setText(QApplication::translate("MainWindow", "DOESN'T CONTROL MOTOR", 0));
	actionFDD_INTFDD_OFF->setToolTip(QApplication::translate("MainWindow", "Machine doesn't control FDD's motor.", 0));
	actionINTHDD_ON->setText(QApplication::translate("MainWindow", "Enable internal HDD", 0));
	actionINTHDD_ON->setToolTip(QApplication::translate("MainWindow", "Enable to use internal hard disk drive(s).", 0));
	actionHEIGHT20->setText(QApplication::translate("MainWindow", "20 lines", 0));
	actionHEIGHT20->setToolTip(QApplication::translate("MainWindow", "Set screen height to 20 lines.\nIf not check, set to 25 lines.", 0));
	actionWIDTH40->setText(QApplication::translate("MainWindow", "40 columns", 0));
	actionWIDTH40->setToolTip(QApplication::translate("MainWindow", "Set screen width to 40 columns.\nIf not check, set to 80 columns.", 0));
	actionTERMINAL->setText(QApplication::translate("MainWindow", "Terminal mode", 0));
	actionTERMINAL->setToolTip(QApplication::translate("MainWindow", "Start with terminal mode.\nIf not check, starting with BASIC mode.", 0));

	actionLT->setText(QApplication::translate("MainWindow", "LT MODE", 0));
	actionLT->setToolTip(QApplication::translate("MainWindow", "Start with PC98LT mode.", 0));

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

#if defined(_PC98DO) || defined(_PC98DOPLUS)
   	actionMemoryWait->setText(QApplication::translate("MainWindow", "Memory Wait", 0));
	actionMemoryWait->setToolTip(QApplication::translate("MainWindow", "Simulate waiting memory.", 0));

	actionCMD_Sing->setText(QApplication::translate("MainWindow", "Support CMD SING", 0));
	actionCMD_Sing->setToolTip(QApplication::translate("MainWindow", "Enable PCM supporting for \"CMD SING\" command.", 0));
	actionPalette->setText(QApplication::translate("MainWindow", "Change palette only within VBLANK.", 0));
	actionPalette->setToolTip(QApplication::translate("MainWindow", "Ignore Palette Changed Outside VBLANK.", 0));
	actionFDD_5Inch->setText(QApplication::translate("MainWindow", "5.25Inch FDD(Need to restart)", 0));
	actionFDD_5Inch->setToolTip(QApplication::translate("MainWindow", "Enable 5.25 inch FDDs.\nThis effects only after restarting this emulator.", 0));

#if defined(SUPPORT_M88_DISKDRV)
	actionM88DRV->setText(QApplication::translate("MainWindow", "M88 DiskDrv(Need to restart)", 0));
	actionM88DRV->setToolTip(QApplication::translate("MainWindow", "Enable M88 stile Disk Drives.\nThis effects only after restarting this emulator.", 0));
#endif
#if defined(SUPPORT_QUASIS88_CMT)
	actionQuasiS88CMT->setText(QApplication::translate("MainWindow", "Enable QUASIS88 CMT", 0));
	actionQuasiS88CMT->setToolTip(QApplication::translate("MainWindow", "Enable loading QuasiS88 style CMT images.", 0));
#endif

#endif
#if defined(USE_PRINTER)
	actionPrintDevice[1]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MainWindow", "NEC PC-PR201 kanji serial printer.", 0));
	actionPrintDevice[1]->setEnabled(false);
	#if (defined(_PC98DO) || defined(_PC98DOPLUS)) && defined(SUPPORT_PC88_JAST)

	actionPrintDevice[2]->setText(QString::fromUtf8("JAST SOUND"));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MenuPC88", "Use JAST SOUND : PCM sound unit.", 0));
	actionPrintDevice[2]->setEnabled(true);
	#endif
#endif

#if defined(SUPPORT_320KB_FDD_IF)
	actionConnect2D->setText(QApplication::translate("MainWindow", "Connect 320KB 2D Drive", 0));
#endif
#if defined(_PC9801) || defined(_PC9801E)
	actionConnect2DD->setText(QApplication::translate("MainWindow", "Connect 2DD Drive", 0));
	actionConnect2HD->setText(QApplication::translate("MainWindow", "Connect 2HD Drive", 0));
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
#elif defined(_PC98DO) || defined(_PC98DOPLUS)
	actionDebugger[0]->setText(QApplication::translate("MainWindow", "PC-98 Main CPU", 0));
	actionDebugger[1]->setText(QApplication::translate("MainWindow", "PC-88 Main CPU", 0));
	actionDebugger[2]->setText(QApplication::translate("MainWindow", "PC-88 Sub CPU", 0));
	actionDebugger[1]->setVisible(true);
	actionDebugger[2]->setVisible(true);
#elif defined(HAS_I286)
	actionDebugger[0]->setText(QApplication::translate("MainWindow", "i80286 Main CPU", 0));
#elif defined(HAS_I386)
	actionDebugger[0]->setText(QApplication::translate("MainWindow", "i80386 Main CPU", 0));
#elif defined(HAS_I486)
	actionDebugger[0]->setText(QApplication::translate("MainWindow", "i80486 Main CPU", 0));
#elif defined(UPPER_I386)
	actionDebugger[0]->setText(QApplication::translate("MainWindow", "i80x86 Main CPU", 0));
#endif
#if defined(HAS_SUB_V30)
	actionDebugger[1]->setText(QApplication::translate("MainWindow", "V30 Sub CPU", 0));
	actionDebugger[1]->setVisible(true);
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
	ConfigCPUTypes(USE_CPU_TYPE);
#endif

#if defined(HAS_SUB_V30)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionSUB_V30, ((0x1) << DIPSWITCH_POSITION_USE_V30), p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionSUB_V30);
#endif
//	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionRAM_512K, ((0x1) << DIPSWITCH_POSITION_RAM512K), p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
//	menuMachine->addAction(actionRAM_512K);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionGDC_FAST, ((0x1) << DIPSWITCH_POSITION_GDC_FAST), p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionGDC_FAST);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionFDD_INTFDD_OFF, ((0x1) << DIPSWITCH_POSITION_INTFDD_OFF), p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionFDD_INTFDD_OFF);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionINTHDD_ON, ((0x1) << DIPSWITCH_POSITION_INTHDD_ON), p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionINTHDD_ON);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT_NEGATIVE(actionINIT_MEMSW, ((0x1) << DIPSWITCH_POSITION_NOINIT_MEMSW), p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch_negative(bool)));
	menuMachine->addAction(actionINIT_MEMSW);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT_NEGATIVE(actionHEIGHT20, ((0x1) << DIPSWITCH_POSITION_TEXT_HEIGHT25), p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch_nagative(bool)));
	menuMachine->addAction(actionHEIGHT20);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT_NEGATIVE(actionWIDTH40, ((0x1) << DIPSWITCH_POSITION_TEXT_WIDTH80), p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch_negative(bool)));
	menuMachine->addAction(actionWIDTH40);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionTERMINAL, ((0x1) << DIPSWITCH_POSITION_TERMINAL_MODE), p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionTERMINAL);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionLT, ((0x1) << DIPSWITCH_POSITION_LT_MODE), p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionLT);

#if defined(SUPPORT_EGC)
	menuMachine->addSeparator();
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionEGC, ((0x1) << DIPSWITCH_POSITION_EGC), p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionEGC);
#endif

#if defined(_PC98DO) || defined(_PC98DOPLUS)
	menuMachine->addSeparator();
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionMemoryWait, DIPSWITCH_MEMWAIT, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionMemoryWait);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionPalette, DIPSWITCH_PALETTE, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionPalette);

	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionFDD_5Inch, DIPSWITCH_5INCH_FDD, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionFDD_5Inch);

#if defined(SUPPORT_M88_DISKDRV)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionM88DRV, DIPSWITCH_M88_DISKDRV, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionM88DRV);
#endif
#if defined(SUPPORT_QUASIS88_CMT)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionQuasiS88CMT, DIPSWITCH_QUASIS88_CMT, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionQuasiS88CMT);
#endif
#if defined(SUPPORT_QUASIS88_CMT) || defined(SUPPORT_M88_DISKDRV)
	menuMachine->addSeparator();
#endif
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionCMD_Sing, DIPSWITCH_CMDSING, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionCMD_Sing);
	menuMachine->addSeparator();
#endif
#if defined(SUPPORT_320KB_FDD_IF)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionConnect2D, DIPSWITCH_2D, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionConnect2D);
#endif
#if defined(_PC9801) || defined(_PC9801E)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionConnect2DD, DIPSWITCH_2DD, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionConnect2DD);
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionConnect2HD, DIPSWITCH_2HD, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionConnect2HD);
#endif
#ifdef USE_BOOT_MODE
	ConfigCPUBootMode(USE_BOOT_MODE);
#endif

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
