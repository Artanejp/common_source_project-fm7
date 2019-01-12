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

extern config_t config;

Object_Menu_Control_88::Object_Menu_Control_88(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_88::~Object_Menu_Control_88()
{
}


void Object_Menu_Control_88::do_set_memory_wait(bool flag)
{
	emit sig_set_dipsw(0, flag);
}


Action_Control_88::Action_Control_88(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	pc88_binds = new Object_Menu_Control_88(parent, p);
	pc88_binds->setValue1(0);
}

Action_Control_88::~Action_Control_88()
{
	delete pc88_binds;
}

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
	config_sound_device_type = config.sound_type;
	
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
		actionSoundDevice[1]->setText(QString::fromUtf8("OPNA"));   
		actionSoundDevice[2]->setText(QString::fromUtf8("OPN + OPNA"));   
		actionSoundDevice[0]->setToolTip(QApplication::translate("MenuPC88", "Using YM2203(OPN) as FM sounder.", 0));
		actionSoundDevice[1]->setToolTip(QApplication::translate("MenuPC88", "Using YM2608(OPNA) as FM sounder.", 0));
		actionSoundDevice[2]->setToolTip(QApplication::translate("MenuPC88", "Using YM2203(OPN) and YM2608(OPNA) as FM sounder.", 0));
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
	#if defined(SUPPORT_OPNA)
		actionSoundDevice[0]->setToolTip(QApplication::translate("MenuPC88", "PC-8801-11 (OPN).", 0));
		actionSoundDevice[0]->setText(QString::fromUtf8("PC-8801-11 (OPN)"));
		actionSoundDevice[1]->setText(QString::fromUtf8("PC-8801-23 (OPNA)"));
		actionSoundDevice[1]->setToolTip(QApplication::translate("MenuPC88", "PC-8801-23 (OPNA).", 0));
	#else
		actionSoundDevice[0]->setToolTip(QApplication::translate("MenuPC88", "PC-8801-11 (OPN).", 0));
		actionSoundDevice[0]->setText(QString::fromUtf8("PC-8801-11 (OPN)"));
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
#endif
	
	actionMemoryWait->setText(QApplication::translate("MenuPC88", "Wait Memory", 0));
	actionMemoryWait->setToolTip(QApplication::translate("MenuPC88", "Simulate waiting memory.", 0));
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
	actionMemoryWait = new Action_Control_88(this, using_flags);
	actionMemoryWait->setCheckable(true);
	actionMemoryWait->setVisible(true);
	actionMemoryWait->setChecked(false);
   
	menuMachine->addAction(actionMemoryWait);
	if((config.dipswitch & 0x0001) != 0) actionMemoryWait->setChecked(true);
	connect(actionMemoryWait, SIGNAL(toggled(bool)),
			actionMemoryWait->pc88_binds, SLOT(do_set_memory_wait(bool)));
	connect(actionMemoryWait->pc88_binds, SIGNAL(sig_set_dipsw(int, bool)),
			this, SLOT(set_dipsw(int, bool)));

}


META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	config_sound_device_type = 0;
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



