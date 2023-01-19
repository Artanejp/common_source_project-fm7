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
#include <QApplication>
#include <QMenu>

#include "../../../src/vm/vm.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"
#include "sound_dialog.h"

//QT_BEGIN_NAMESPACE

void META_MainWindow::retranslateVolumeLabels(Ui_SoundDialog *p)
{
	if(p != NULL) {
		p->setDeviceLabel(1, QApplication::translate("MenuPC88", "CMT", 0));
		switch(config_sound_device_type) {
		case 0:
			p->setDeviceLabel(2, QApplication::translate("MenuPC88", "OPNA", 0));
			p->setSliderVisible(2, true);
			p->setSliderVisible(3, false);
			break;
		case 1:
			p->setDeviceLabel(2, QApplication::translate("MenuPC88", "OPN", 0));
			p->setSliderVisible(2, true);
			p->setSliderVisible(3, false);
			break;
#ifdef SUPPORT_PC88_SB2
		case 2:
			p->setDeviceLabel(2, QApplication::translate("MenuPC88", "OPN", 0));
			p->setDeviceLabel(3, QApplication::translate("MenuPC88", "OPNA", 0));
			p->setSliderVisible(2, true);
			p->setSliderVisible(3, true);
			break;
		case 3:
			p->setDeviceLabel(2, QApplication::translate("MenuPC88", "OPN-1", 0));
			p->setDeviceLabel(3, QApplication::translate("MenuPC88", "OPN-2", 0));
			p->setSliderVisible(2, true);
			p->setSliderVisible(3, true);
			break;
		case 4:
			p->setDeviceLabel(2, QApplication::translate("MenuPC88", "OPNA-1", 0));
			p->setDeviceLabel(3, QApplication::translate("MenuPC88", "OPNA-2", 0));
			p->setSliderVisible(2, true);
			p->setSliderVisible(3, true);
			break;
		case 5:
			p->setDeviceLabel(2, QApplication::translate("MenuPC88", "OPNA", 0));
			p->setDeviceLabel(3, QApplication::translate("MenuPC88", "OPN", 0));
			p->setSliderVisible(2, true);
			p->setSliderVisible(3, true);
			break;
#endif
		}
		
	}
}

void META_MainWindow::retranslateUi(void)
{
	const char *title="";
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(title, false);
	config_sound_device_type = p_config->sound_type;
	
	this->setWindowTitle(QApplication::translate("MenuPC88", "MainWindow", 0));
	
	// PC88 Specified
	menuCpuType->setTitle(QApplication::translate("MenuPC88", "CPU Frequency", 0));
#ifdef SUPPORT_PC88_HIGH_CLOCK
	actionCpuType[0]->setText(QString::fromUtf8("8MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("4MHz"));
	actionCpuType[2]->setText(QString::fromUtf8("8MHz (FE2/MC)"));
#else // _PC8001SR
	actionCpuType[0]->setText(QString::fromUtf8("4MHz"));
#if defined(USE_CPU_TYPE)
	for(int i = 1; i < USE_CPU_TYPE; i++) {
		actionCpuType[i]->setVisible(false);
	}
#endif
//	menuCpuType->setVisible(false);
//	actionCpuType[0]->setVisible(false);
#endif

	menuBootMode->setTitle(QApplication::translate("MenuPC88", "Machine Mode", 0));
	menuBootMode->setToolTipsVisible(true);
#if defined(_PC8801)
	actionBootMode[0]->setText(QString::fromUtf8("N88 Mode"));
	actionBootMode[3]->setText(QString::fromUtf8("N Mode (N80 compatible)"));
	actionBootMode[0]->setToolTip(QApplication::translate("MenuPC88", "N88 Mode.\nYou can run softwares of PC-8801/mk2.", 0));
	actionBootMode[3]->setToolTip(QApplication::translate("MenuPC88", "N Mode.\nYou can run softwares of PC-8001/mk2.", 0));
	actionBootMode[1]->setVisible(false);
	actionBootMode[2]->setVisible(false);
#elif defined(_PC8801MK2)
	actionBootMode[0]->setText(QString::fromUtf8("N88 Mode"));
	actionBootMode[3]->setText(QString::fromUtf8("N Mode (N80 compatible)"));
	actionBootMode[0]->setToolTip(QApplication::translate("MenuPC88", "N88 Mode.\nYou can run softwares of PC-8801/mk2.", 0));
	actionBootMode[3]->setToolTip(QApplication::translate("MenuPC88", "N Mode.\nYou can run softwares of PC-8001/mk2.", 0));
	actionBootMode[1]->setVisible(false);
	actionBootMode[2]->setVisible(false);
	
#elif defined(_PC8801MA)
	actionBootMode[0]->setText(QString::fromUtf8("N88-V1(S) Mode"));
	actionBootMode[1]->setText(QString::fromUtf8("N88-V1(H) Mode"));	
	actionBootMode[2]->setText(QString::fromUtf8("N88-V2 Mode"));
	actionBootMode[3]->setText(QString::fromUtf8("N Mode (N80 compatible)"));
	actionBootMode[0]->setToolTip(QApplication::translate("MenuPC88", "V1(Standard) Mode.\nYou can run softwares of PC-8801/mk2.", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("MenuPC88", "V1(High Speed) Mode.\nYou can run softwares of PC-8801/mk2 faster.", 0));	
	actionBootMode[2]->setToolTip(QApplication::translate("MenuPC88", "V2 Mode.\nYou can run only softwares for PC-8801SR or later.", 0));
	actionBootMode[3]->setToolTip(QApplication::translate("MenuPC88", "N Mode.\nYou can run softwares of PC-8001/mk2.", 0));
#elif defined(_PC8001)
	menuBootMode->setVisible(false);
	menuBootMode->setToolTipsVisible(false);
	actionBootMode[0]->setVisible(false);
	actionBootMode[1]->setVisible(false);
	actionBootMode[2]->setVisible(false);
#elif defined(_PC8001MK2)
	actionBootMode[0]->setText(QString::fromUtf8("N80     Mode"));
	actionBootMode[2]->setText(QString::fromUtf8("N Mode"));
	actionBootMode[0]->setToolTip(QApplication::translate("MenuPC88", "N80 Mode.\nYou can run softwares of PC-8001/mk2.", 0));
	actionBootMode[2]->setToolTip(QApplication::translate("MenuPC88", "N  Mode.\nYou can run only softwares for PC-8001.", 0));
	actionBootMode[1]->setVisible(false);
#elif defined(_PC8001SR)
	actionBootMode[0]->setText(QString::fromUtf8("N80     Mode"));
	actionBootMode[1]->setText(QString::fromUtf8("N80-V2(SR) Mode"));	
	actionBootMode[2]->setText(QString::fromUtf8("N Mode"));
	actionBootMode[0]->setToolTip(QApplication::translate("MenuPC88", "N80 Mode.\nYou can run softwares of PC-8001/mk2.", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("MenuPC88", "N80 V2 Mode.\nYou can run only softwares for PC-8001mk2SR or later.", 0));
	actionBootMode[2]->setToolTip(QApplication::translate("MenuPC88", "N  Mode.\nYou can run only softwares for PC-8001.", 0));
#endif

#if defined(SUPPORT_PC88_OPN1) && defined(SUPPORT_PC88_OPN2)
	#if defined(_PC8001SR)
		menuSoundDevice->setTitle(QApplication::translate("MenuPC88", "Sound Board", 0));
		actionSoundDevice[0]->setText(QString::fromUtf8("OPN"));
		actionSoundDevice[1]->setText(QString::fromUtf8("OPN + OPN"));   
		actionSoundDevice[0]->setToolTip(QApplication::translate("MenuPC88", "Using YM2203(OPN) as FM sounder.", 0));
		actionSoundDevice[1]->setToolTip(QApplication::translate("MenuPC88", "Using Twin YM2203(OPN) as FM sounder.", 0));
	#else
		menuSoundDevice->setTitle(QApplication::translate("MenuPC88", "Sound Board", 0));
		actionSoundDevice[0]->setText(QString::fromUtf8("PC-8801-23 (OPNA)"));
		actionSoundDevice[1]->setText(QString::fromUtf8("PC-8801-11 (OPN)"));   
		actionSoundDevice[2]->setText(QString::fromUtf8("Sound Board 2 (OPN + OPNA)"));   
		actionSoundDevice[3]->setText(QString::fromUtf8("Sound Board 2(OPN + OPN)"));   
		actionSoundDevice[4]->setText(QString::fromUtf8("Sound Board 2 (OPNA + OPNA)"));   
		actionSoundDevice[5]->setText(QString::fromUtf8("Sound Board 2 (OPNA + OPN)"));
		actionSoundDevice[0]->setToolTip(QApplication::translate("MenuPC88", "PC-8801-23 (OPNA).", 0));
		actionSoundDevice[1]->setToolTip(QApplication::translate("MenuPC88", "PC-8801-11 (OPN).", 0));   
		actionSoundDevice[2]->setToolTip(QApplication::translate("MenuPC88", "Sound Board 2 (OPN + OPNA).", 0));   
		actionSoundDevice[3]->setToolTip(QApplication::translate("MenuPC88", "Sound Board 2 (OPN + OPN).", 0));   
		actionSoundDevice[4]->setToolTip(QApplication::translate("MenuPC88", "Sound Board 2 (OPNA + OPNA).", 0));   
		actionSoundDevice[5]->setToolTip(QApplication::translate("MenuPC88", "Sound Board 2 (OPNA + OPN).", 0));
	#endif
#elif defined(SUPPORT_PC88_OPN1) || defined(SUPPORT_PC88_OPN2)
	menuSoundDevice->setTitle(QApplication::translate("MenuPC88", "Sound Board", 0));

	actionSoundDevice[0]->setText(QString::fromUtf8("None"));
	actionSoundDevice[0]->setToolTip(QApplication::translate("MenuPC88", "None.", 0));
	#if defined(SUPPORT_OPNA)
		actionSoundDevice[1]->setText(QString::fromUtf8("PC-8801-23 (OPNA)"));
		actionSoundDevice[1]->setToolTip(QApplication::translate("MenuPC88", "PC-8801-23 (OPNA).", 0));
	#else
		actionSoundDevice[1]->setText(QString::fromUtf8("PC-8801-11 (OPN)"));
		actionSoundDevice[1]->setToolTip(QApplication::translate("MenuPC88", "PC-8801-11 (OPN).", 0));
	#endif
#endif
#ifdef USE_DEBUGGER
	actionDebugger[0]->setText(QApplication::translate("MenuPC88", "Main CPU", 0));
#if defined(_PC8001SR)	
	actionDebugger[1]->setText(QApplication::translate("MenuPC88", "PC-80S31K CPU", 0));
#else
	actionDebugger[1]->setText(QApplication::translate("MenuPC88", "Sub CPU", 0));
#endif	
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
	
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif	
#if defined(USE_JOYSTICK_TYPE)
	actionJoystickType[0]->setText(QApplication::translate("MenuPC88", "Joystick", 0));
	actionJoystickType[1]->setText(QApplication::translate("MenuPC88", "Bus Mouse", 0));
	actionJoystickType[0]->setToolTip(QApplication::translate("MenuPC88", "Connect joystick to JOY PORT.", 0));
	actionJoystickType[1]->setToolTip(QApplication::translate("MenuPC88", "Connect bus-mouse to JOY PORT.", 0));
	menuJoystickType->setTitle(QApplication::translate("MenuPC88", "Joy Port", 0));
#endif
#if defined(USE_PRINTER)
	actionPrintDevice[1]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MenuPC88", "NEC PC-PR201 kanji serial printer.", 0));
	actionPrintDevice[1]->setEnabled(false);
	
	#if defined(SUPPORT_PC88_JAST)
	actionPrintDevice[2]->setText(QString::fromUtf8("JAST SOUND"));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MenuPC88", "Use JAST SOUND : PCM sound unit.", 0));
	actionPrintDevice[2]->setEnabled(true);
	#endif
#endif

	
#if defined(_PC8001SR) || defined(PC8801SR_VARIANT)
	actionMemoryWait->setText(QApplication::translate("MenuPC88", "Wait Memory", 0));
	actionMemoryWait->setToolTip(QApplication::translate("MenuPC88", "Simulate waiting memory.", 0));
#endif

#if defined(SUPPORT_QUASIS88_CMT)
	actionQuasiS88CMT->setText(QApplication::translate("MenuPC88", "Enable QUASIS88 CMT", 0));
	actionQuasiS88CMT->setToolTip(QApplication::translate("MenuPC88", "Enable loading QuasiS88 style CMT images.", 0));
#endif
#if defined(PC8801_VARIANT)
	actionCMD_Sing->setText(QApplication::translate("MenuPC88", "Support CMD SING", 0));
	actionCMD_Sing->setToolTip(QApplication::translate("MenuPC88", "Enable PCM supporting for \"CMD SING\" command.", 0));
#endif
	actionPalette->setText(QApplication::translate("MenuPC88", "Change palette only within VBLANK.", 0));
	actionPalette->setToolTip(QApplication::translate("MenuPC88", "Ignore Palette Changed Outside VBLANK.", 0));
	
	actionFDD_5Inch->setText(QApplication::translate("MenuPC88", "5.25Inch FDD(Need to restart)", 0));
	actionFDD_5Inch->setToolTip(QApplication::translate("MenuPC88", "Enable 5.25 inch FDDs.\nThis effects only after restarting this emulator.", 0));
#if defined(SUPPORT_PC88_FDD_8INCH)
	actionFDD_8Inch->setText(QApplication::translate("MenuPC88", "8Inch FDD(Need to restart)", 0));
	actionFDD_8Inch->setToolTip(QApplication::translate("MenuPC88", "Enable 8 inch FDDs for 3: and 4:.\nThis effects only after restarting this emulator.", 0));
#endif
#if defined(SUPPORT_M88_DISKDRV)
	actionM88DRV->setText(QApplication::translate("MenuPC88", "M88 DiskDrv(Need to restart)", 0));
	actionM88DRV->setToolTip(QApplication::translate("MenuPC88", "Enable M88 stile Disk Drives.\nThis effects only after restarting this emulator.", 0));
#endif
	
#if defined(SUPPORT_PC88_HMB20)
	actionHMB20->setText(QApplication::translate("MenuPC88", "Use HMB20(Need RESTART)", 0));
	actionHMB20->setToolTip(QApplication::translate("MenuPC88", "Using HMB20 OPM sound board.\nRe-start emulator when changed.", 0));
#endif

#if defined(SUPPORT_PC88_GSX8800)
	actionGSX8800->setText(QApplication::translate("MenuPC88", "Use GSX8800(Need RESTART)", 0));
	actionGSX8800->setToolTip(QApplication::translate("MenuPC88", "Using GSX8800 PSGs sound board.\nRe-start emulator when changed.", 0));
#endif
#if defined(SUPPORT_PC88_PCG8100)
	actionPCG8100->setText(QApplication::translate("MenuPC88", "Use PCG8100(Need RESTART)", 0));
	actionPCG8100->setToolTip(QApplication::translate("MenuPC88", "Using PCG8100 programmable character generator board.\nRe-start emulator when changed.", 0));
#endif


#if defined(USE_MONITOR_TYPE)
	actionMonitorType[0]->setText(QApplication::translate("MenuPC88", "High Resolution", 0));
	actionMonitorType[1]->setText(QApplication::translate("MenuPC88", "Standard", 0));
#endif
// End.
   // Set Labels
  
} // retranslateUi


void META_MainWindow::setupUI_Emu(void)
{
#if defined(USE_CPU_TYPE)
	ConfigCPUTypes(USE_CPU_TYPE);
#endif

#if defined(PC8001_VARIANT)
	ConfigCPUBootMode(3);
#else
	ConfigCPUBootMode(4);
#endif

#if defined(_PC8001SR) || defined(PC8801SR_VARIANT)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionMemoryWait, DIPSWITCH_MEMWAIT, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionMemoryWait);
#endif
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionPalette, DIPSWITCH_PALETTE, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	
	menuMachine->addAction(actionPalette);
	menuMachine->addSeparator();

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
	
#if defined(PC8801_VARIANT)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionCMD_Sing, DIPSWITCH_CMDSING, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionCMD_Sing);
	menuMachine->addSeparator();
#endif
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionFDD_5Inch, DIPSWITCH_FDD_5INCH, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionFDD_5Inch);
	
#if defined(SUPPORT_PC88_FDD_8INCH)
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionFDD_8Inch, DIPSWITCH_FDD_8INCH, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionFDD_8Inch);
#endif
	menuMachine->addSeparator();
#ifdef SUPPORT_PC88_HMB20
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionHMB20, DIPSWITCH_HMB20, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionHMB20);
	
#endif	
#ifdef SUPPORT_PC88_GSX8800
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionGSX8800, DIPSWITCH_GSX8800, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionGSX8800);
#endif
#ifdef SUPPORT_PC88_PCG8100
	SET_ACTION_SINGLE_DIPSWITCH_CONNECT(actionPCG8100, DIPSWITCH_PCG8100, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
	menuMachine->addAction(actionPCG8100);
#endif
}


META_MainWindow::META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	config_sound_device_type = 0;
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



