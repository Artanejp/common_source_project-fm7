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
		p->setDeviceLabel(1, QApplication::translate("MainWindow", "CMT", 0));
		switch(config_sound_device_type) {
		case 0:
			p->setDeviceLabel(2, QApplication::translate("MainWindow", "OPNA", 0));
			p->setSliderVisible(2, true);
			p->setSliderVisible(3, false);
			break;
		case 1:
			p->setDeviceLabel(2, QApplication::translate("MainWindow", "OPN", 0));
			p->setSliderVisible(2, true);
			p->setSliderVisible(3, false);
			break;
#ifdef SUPPORT_PC88_SB2
		case 2:
			p->setDeviceLabel(2, QApplication::translate("MainWindow", "OPN", 0));
			p->setDeviceLabel(3, QApplication::translate("MainWindow", "OPNA", 0));
			p->setSliderVisible(2, true);
			p->setSliderVisible(3, true);
			break;
		case 3:
			p->setDeviceLabel(2, QApplication::translate("MainWindow", "OPN-1", 0));
			p->setDeviceLabel(3, QApplication::translate("MainWindow", "OPN-2", 0));
			p->setSliderVisible(2, true);
			p->setSliderVisible(3, true);
			break;
		case 4:
			p->setDeviceLabel(2, QApplication::translate("MainWindow", "OPNA-1", 0));
			p->setDeviceLabel(3, QApplication::translate("MainWindow", "OPNA-2", 0));
			p->setSliderVisible(2, true);
			p->setSliderVisible(3, true);
			break;
		case 5:
			p->setDeviceLabel(2, QApplication::translate("MainWindow", "OPNA", 0));
			p->setDeviceLabel(3, QApplication::translate("MainWindow", "OPN", 0));
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
	retranslateControlMenu(title, false);
	retranslateFloppyMenu(0, 1);
	retranslateFloppyMenu(1, 2);
	retranslateCMTMenu(0);
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
	config_sound_device_type = config.sound_type;
	
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
	
	// PC88 Specified
#if defined(_PC8801MA)
	menuCpuType->setTitle("CPU Frequency");
	actionCpuType[0]->setText(QString::fromUtf8("8MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("4MHz"));
	actionCpuType[2]->setText(QString::fromUtf8("8MHz (FE2/MC)"));
#else // _PC8001SR
	menuCpuType->setTitle("CPU Frequency");
	actionCpuType[0]->setText(QString::fromUtf8("4MHz"));
	//menuCpuType->setVisible(false);
	//actionCpuType[0]->setVisible(false);
#endif
  
#if defined(_PC8801MA)
	menuBootMode->setTitle("Machine Mode");
	menuBootMode->setToolTipsVisible(true);
	actionBootMode[0]->setText(QString::fromUtf8("N88-V1(S) Mode"));
	actionBootMode[1]->setText(QString::fromUtf8("N88-V1(H) Mode"));	
	actionBootMode[2]->setText(QString::fromUtf8("N88-V2 Mode"));
	actionBootMode[3]->setText(QString::fromUtf8("N Mode (N80 compatible)"));
	actionBootMode[0]->setToolTip(QApplication::translate("MainWindow", "V1(Standard) Mode.\nYou can run softwares of PC-8801/mk2.", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("MainWindow", "V1(High Speed) Mode.\nYou can run softwares of PC-8801/mk2 faster.", 0));	
	actionBootMode[2]->setToolTip(QApplication::translate("MainWindow", "V2 Mode.\nYou can run only softwares for PC-8801SR or later.", 0));
	actionBootMode[3]->setToolTip(QApplication::translate("MainWindow", "N Mode.\nYou can run softwares of PC-8001/mk2.", 0));
#elif defined(_PC8001SR)
	menuBootMode->setTitle("Machine Mode");
	menuBootMode->setToolTipsVisible(true);
	actionBootMode[0]->setText(QString::fromUtf8("N80-V1     Mode"));
	actionBootMode[1]->setText(QString::fromUtf8("N80-V2(SR) Mode"));	
	actionBootMode[2]->setText(QString::fromUtf8("N Mode"));
	actionBootMode[0]->setToolTip(QApplication::translate("MainWindow", "V1 Mode.\nYou can run softwares of PC-8001/mk2.", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("MainWindow", "V2 Mode.\nYou can run only softwares for PC-8001mk2SR or later.", 0));
	actionBootMode[2]->setToolTip(QApplication::translate("MainWindow", "N  Mode.\nYou can run only softwares for PC-8001.", 0));
#endif

	menuSoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Boards", 0));
#if defined(SUPPORT_PC88_SB2)
	menuSoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Board", 0));
	actionSoundDevice[0]->setText(QString::fromUtf8("PC-8801-23 (OPNA)"));
	actionSoundDevice[1]->setText(QString::fromUtf8("PC-8801-11 (OPN)"));   
	actionSoundDevice[2]->setText(QString::fromUtf8("Sound Board 2 (OPN + OPNA)"));   
	actionSoundDevice[3]->setText(QString::fromUtf8("Sound Board 2(OPN + OPN)"));   
	actionSoundDevice[4]->setText(QString::fromUtf8("Sound Board 2 (OPNA + OPNA)"));   
	actionSoundDevice[5]->setText(QString::fromUtf8("Sound Board 2 (OPNA + OPN)"));
	actionSoundDevice[0]->setToolTip(QApplication::translate("MainWindow", "PC-8801-23 (OPNA).", 0));
	actionSoundDevice[1]->setToolTip(QApplication::translate("MainWindow", "PC-8801-11 (OPN).", 0));   
	actionSoundDevice[2]->setToolTip(QApplication::translate("MainWindow", "Sound Board 2 (OPN + OPNA).", 0));   
	actionSoundDevice[3]->setToolTip(QApplication::translate("MainWindow", "Sound Board 2 (OPN + OPN).", 0));   
	actionSoundDevice[4]->setToolTip(QApplication::translate("MainWindow", "Sound Board 2 (OPNA + OPNA).", 0));   
	actionSoundDevice[5]->setToolTip(QApplication::translate("MainWindow", "Sound Board 2 (OPNA + OPN).", 0));   
#elif defined(SUPPORT_PC88_OPNA)
	menuSoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Board", 0));
	actionSoundDevice[0]->setText(QString::fromUtf8("PC-8801-23 (OPNA)"));
	actionSoundDevice[1]->setText(QString::fromUtf8("PC-8801-11 (OPN)"));
	actionSoundDevice[0]->setToolTip(QApplication::translate("MainWindow", "PC-8801-23 (OPNA).", 0));
	actionSoundDevice[1]->setToolTip(QApplication::translate("MainWindow", "PC-8801-11 (OPN).", 0));
#endif
#ifdef USE_DEBUGGER
	actionDebugger[0]->setText(QApplication::translate("MainWindow", "Main CPU", 0));
#if defined(_PC8001SR)	
	actionDebugger[1]->setText(QApplication::translate("MainWindow", "PC-80S31K CPU", 0));
#else
	actionDebugger[1]->setText(QApplication::translate("MainWindow", "Sub CPU", 0));
#endif	
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
	
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif	
#if defined(USE_JOYSTICK_TYPE)
	actionJoystickType[0]->setText(QApplication::translate("MainWindow", "Joystick", 0));
	actionJoystickType[1]->setText(QApplication::translate("MainWindow", "Bus Mouse", 0));
	actionJoystickType[0]->setToolTip(QApplication::translate("MainWindow", "Connect joystick to JOY PORT.", 0));
	actionJoystickType[1]->setToolTip(QApplication::translate("MainWindow", "Connect bus-mouse to JOY PORT.", 0));
	menuJoystickType->setTitle(QApplication::translate("MainWindow", "Joy Port", 0));
#endif
#if defined(USE_PRINTER)
	actionPrintDevice[1]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MainWindow", "NEC PC-PR201 kanji serial printer.", 0));
#endif	
	actionMemoryWait->setText(QApplication::translate("MainWindow", "Wait Memory", 0));
	actionMemoryWait->setToolTip(QApplication::translate("MainWindow", "Simulate waiting memory.", 0));
// End.
   // Set Labels
  
} // retranslateUi


void META_MainWindow::setupUI_Emu(void)
{
	menuCpuType = new QMenu(menuMachine);
	menuCpuType->setObjectName(QString::fromUtf8("menuControl_CpuType"));
#if defined(_PC8801MA)
	ConfigCPUTypes(3);
#else
	ConfigCPUTypes(1);
#endif
	menuMachine->addAction(menuCpuType->menuAction());

	menuBootMode = new QMenu(menuMachine);
	menuBootMode->setObjectName(QString::fromUtf8("menuControl_BootMode"));
	menuMachine->addAction(menuBootMode->menuAction());
#if defined(_PC8801MA)
	ConfigCPUBootMode(4);
#elif defined(_PC8001SR)
	ConfigCPUBootMode(3);
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



