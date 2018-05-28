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

void Object_Menu_Control_88::do_set_display_mode(void)
{
	emit sig_display_mode(getValue1());
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
#if defined(_PC8801MA)
	actionCpuType[0]->setText(QString::fromUtf8("8MHz"));
	actionCpuType[1]->setText(QString::fromUtf8("4MHz"));
	actionCpuType[2]->setText(QString::fromUtf8("8MHz (FE2/MC)"));
#else // _PC8001SR
	actionCpuType[0]->setText(QString::fromUtf8("4MHz"));
	//menuCpuType->setVisible(false);
	//actionCpuType[0]->setVisible(false);
#endif
  
#if defined(_PC8801MA)
	menuBootMode->setTitle(QApplication::translate("MenuPC88", "Machine Mode", 0));
	menuBootMode->setToolTipsVisible(true);
	actionBootMode[0]->setText(QString::fromUtf8("N88-V1(S) Mode"));
	actionBootMode[1]->setText(QString::fromUtf8("N88-V1(H) Mode"));	
	actionBootMode[2]->setText(QString::fromUtf8("N88-V2 Mode"));
	actionBootMode[3]->setText(QString::fromUtf8("N Mode (N80 compatible)"));
	actionBootMode[0]->setToolTip(QApplication::translate("MenuPC88", "V1(Standard) Mode.\nYou can run softwares of PC-8801/mk2.", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("MenuPC88", "V1(High Speed) Mode.\nYou can run softwares of PC-8801/mk2 faster.", 0));	
	actionBootMode[2]->setToolTip(QApplication::translate("MenuPC88", "V2 Mode.\nYou can run only softwares for PC-8801SR or later.", 0));
	actionBootMode[3]->setToolTip(QApplication::translate("MenuPC88", "N Mode.\nYou can run softwares of PC-8001/mk2.", 0));
#elif defined(_PC8001SR)
	menuBootMode->setTitle("Machine Mode");
	menuBootMode->setToolTipsVisible(true);
	actionBootMode[0]->setText(QString::fromUtf8("N80-V1     Mode"));
	actionBootMode[1]->setText(QString::fromUtf8("N80-V2(SR) Mode"));	
	actionBootMode[2]->setText(QString::fromUtf8("N Mode"));
	actionBootMode[0]->setToolTip(QApplication::translate("MenuPC88", "V1 Mode.\nYou can run softwares of PC-8001/mk2.", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("MenuPC88", "V2 Mode.\nYou can run only softwares for PC-8001mk2SR or later.", 0));
	actionBootMode[2]->setToolTip(QApplication::translate("MenuPC88", "N  Mode.\nYou can run only softwares for PC-8001.", 0));
#endif

	menuSoundDevice->setTitle(QApplication::translate("MenuPC88", "Sound Boards", 0));
#if defined(SUPPORT_PC88_SB2)
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
#elif defined(SUPPORT_PC88_OPNA)
	menuSoundDevice->setTitle(QApplication::translate("MenuPC88", "Sound Board", 0));
	actionSoundDevice[0]->setText(QString::fromUtf8("PC-8801-23 (OPNA)"));
	actionSoundDevice[1]->setText(QString::fromUtf8("PC-8801-11 (OPN)"));
	actionSoundDevice[0]->setToolTip(QApplication::translate("MenuPC88", "PC-8801-23 (OPNA).", 0));
	actionSoundDevice[1]->setToolTip(QApplication::translate("MenuPC88", "PC-8801-11 (OPN).", 0));
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
	if(actionSoundPlayTape != NULL) actionSoundPlayTape->setEnabled(false);
	if(action_SoundFilesRelay != NULL) action_SoundFilesRelay->setEnabled(false);
	
	actionMemoryWait->setText(QApplication::translate("MenuPC88", "Wait Memory", 0));
	actionMemoryWait->setToolTip(QApplication::translate("MenuPC88", "Simulate waiting memory.", 0));
#if defined(USE_MONITOR_TYPE)
	menu_Emu_DisplayMode->setTitle(QApplication::translate("MenuPC88", "Display Mode", 0));
	action_Emu_DisplayMode[0]->setText(QApplication::translate("MenuPC88", "High Resolution", 0));
	action_Emu_DisplayMode[1]->setText(QApplication::translate("MenuPC88", "Standard", 0));
#endif
// End.
   // Set Labels
  
} // retranslateUi


void META_MainWindow::setupUI_Emu(void)
{
#if defined(USE_CPU_TYPE)
	ConfigCPUTypes(USE_CPU_TYPE);
#endif

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
#if defined(USE_MONITOR_TYPE)
   menu_Emu_DisplayMode = new QMenu(menuMachine);
   menu_Emu_DisplayMode->setObjectName(QString::fromUtf8("menu_DisplayMode"));
   
   actionGroup_DisplayMode = new QActionGroup(this);
   actionGroup_DisplayMode->setObjectName(QString::fromUtf8("actionGroup_DisplayMode"));
   actionGroup_DisplayMode->setExclusive(true);
   menuMachine->addAction(menu_Emu_DisplayMode->menuAction());   
   for(int i = 0; i < USE_MONITOR_TYPE; i++) {
	   action_Emu_DisplayMode[i] = new Action_Control_88(this, using_flags);
	   action_Emu_DisplayMode[i]->setCheckable(true);
	   action_Emu_DisplayMode[i]->pc88_binds->setValue1(i);
	   if(i == config.monitor_type) action_Emu_DisplayMode[i]->setChecked(true); // Need to write configure
   }
   action_Emu_DisplayMode[0]->setObjectName(QString::fromUtf8("action_Emu_DisplayMode_High"));
   action_Emu_DisplayMode[1]->setObjectName(QString::fromUtf8("action_Emu_DisplayMode_Standard"));
   for(int i = 0; i < 2; i++) {
	   menu_Emu_DisplayMode->addAction(action_Emu_DisplayMode[i]);
	   actionGroup_DisplayMode->addAction(action_Emu_DisplayMode[i]);
	   connect(action_Emu_DisplayMode[i], SIGNAL(triggered()),
			   action_Emu_DisplayMode[i]->pc88_binds, SLOT(do_set_display_mode()));
	   connect(action_Emu_DisplayMode[i]->pc88_binds, SIGNAL(sig_display_mode(int)),
			   this, SLOT(set_monitor_type(int)));
   }
#endif

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



