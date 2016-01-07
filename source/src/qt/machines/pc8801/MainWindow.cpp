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


Object_Menu_Control_88::Object_Menu_Control_88(QObject *parent) : Object_Menu_Control(parent)
{
}

Object_Menu_Control_88::~Object_Menu_Control_88()
{
}



void Object_Menu_Control_88::do_set_memory_wait(bool flag)
{
	emit sig_set_dipsw(0, flag);
}


Action_Control_88::Action_Control_88(QObject *parent) : Action_Control(parent)
{
	pc88_binds = new Object_Menu_Control_88(parent);
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
	retranslateCMTMenu();
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateUI_Help();
	config_sound_device_type = config.sound_device_type;
	
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
	
	actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0));
	
	actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));
	
	
	//	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0));
	//      actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0));
	// 
	menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0));
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
	actionBootMode[0]->setText(QString::fromUtf8("N88-V1(S) Mode"));
	actionBootMode[1]->setText(QString::fromUtf8("N88-V1(H) Mode"));	
	actionBootMode[2]->setText(QString::fromUtf8("N88-V2 Mode"));
	actionBootMode[3]->setText(QString::fromUtf8("N Mode (N80 compatible)"));
#elif defined(_PC8001SR)
	menuBootMode->setTitle("Machine Mode");
	actionBootMode[0]->setText(QString::fromUtf8("N80-V1     Mode"));
	actionBootMode[1]->setText(QString::fromUtf8("N80-V2(SR) Mode"));	
	actionBootMode[2]->setText(QString::fromUtf8("N Mode"));
#endif
  
#if defined(SUPPORT_PC88_SB2)
	menuSoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Board", 0));
	actionSoundDevice[0]->setText(QString::fromUtf8("PC-8801-23 (OPNA)"));
	actionSoundDevice[1]->setText(QString::fromUtf8("PC-8801-11 (OPN)"));   
	actionSoundDevice[2]->setText(QString::fromUtf8("Sound Board 2 (OPN + OPNA)"));   
#elif defined(SUPPORT_PC88_OPNA)
	menuSoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Board", 0));
	actionSoundDevice[0]->setText(QString::fromUtf8("PC-8801-23 (OPNA)"));
	actionSoundDevice[1]->setText(QString::fromUtf8("PC-8801-11 (OPN)"));
#endif
#ifdef USE_DEBUGGER
	actionDebugger_1->setText(QApplication::translate("MainWindow", "Main CPU", 0));
	actionDebugger_2->setText(QApplication::translate("MainWindow", "Sub  CPU", 0));
	actionDebugger_3->setVisible(false);
#endif	
#if defined(USE_DEVICE_TYPE)
	actionDeviceType[0]->setText(QApplication::translate("MainWindow", "Joystick", 0));
	actionDeviceType[1]->setText(QApplication::translate("MainWindow", "Bus Mouse", 0));
	menuDeviceType->setTitle(QApplication::translate("MainWindow", "Joy Port", 0));
#endif
	actionMemoryWait->setText(QApplication::translate("MainWindow", "Wait Memory", 0));
// End.
// 
//        menuRecord->setTitle(QApplication::translate("MainWindow", "Record", 0));
//        menuRecoad_as_movie->setTitle(QApplication::translate("MainWindow", "Recoad as movie", 0));
	
	menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0));
	menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0));
  
	menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0));
	actionHelp_AboutQt->setText(QApplication::translate("MainWindow", "About Qt", 0));
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
	actionMemoryWait = new Action_Control_88(this);
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


META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
	config_sound_device_type = 0;
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



