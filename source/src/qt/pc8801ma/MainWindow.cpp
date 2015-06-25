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
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE


Object_Menu_Control_88::Object_Menu_Control_88(QObject *parent) : Object_Menu_Control(parent)
{
}

Object_Menu_Control_88::~Object_Menu_Control_88()
{
}

void Object_Menu_Control_88::do_set_sound_device(void)
{
   emit sig_sound_device(this->getValue1());
}

void Object_Menu_Control_88::do_set_device_type(void)
{
   emit sig_device_type(this->getValue1());
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

   
void META_MainWindow::do_set_sound_device(int num)
{
   if((num < 0) || (num >= 2)) return;
#ifdef USE_SOUND_DEVICE_TYPE
   if(emu) {
      config.sound_device_type = num;
      emu->LockVM();
      emu->update_config();
      emu->UnlockVM();
   }
#endif
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
   
  this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
  
  
  actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0));
  
  actionAbout->setText(QApplication::translate("MainWindow", "About...", 0));
  

   //	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0));
  //      actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0));
  // 
  menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0));
  menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0));
  // PC88 Specified
  menuCpuType->setTitle("CPU Frequency");
  actionCpuType[0]->setText(QString::fromUtf8("8MHz"));
  actionCpuType[1]->setText(QString::fromUtf8("4MHz"));

#if defined(_PC8801MA)
  menuBootMode->setTitle("Machine Mode");
  actionBootMode[0]->setText(QString::fromUtf8("N88-V1(S) Mode"));
  actionBootMode[1]->setText(QString::fromUtf8("N88-V1(H) Mode"));	
  actionBootMode[2]->setText(QString::fromUtf8("N88-V2 Mode"));
  actionBootMode[3]->setText(QString::fromUtf8("N Mode (N80 compatible)"));
#elif defined(_PC8001SR)
  menuBootMode->setTitle("Machine Mode");
  actionBootMode[0]->setText(QString::fromUtf8("N80-V1 Mode"));
  actionBootMode[1]->setText(QString::fromUtf8("N80-V2 Mode"));	
  actionBootMode[2]->setText(QString::fromUtf8("N Mode"));
#endif

#if defined(SUPPORT_PC88_OPNA)
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
   // Set Labels
  
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
   menuCpuType = new QMenu(menuMachine);
   menuCpuType->setObjectName(QString::fromUtf8("menuControl_CpuType"));
   menuMachine->addAction(menuCpuType->menuAction());
   ConfigCPUTypes(2);
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
	menuMachine->addAction(actionMemoryWait);
	if((config.dipswitch & 0x0001) != 0) actionMemoryWait->setChecked(true);
	connect(actionMemoryWait, SIGNAL(triggered()),
		actionMemoryWait->pc88_binds, SLOT(do_set_memory_wait(bool)));
	connect(actionMemoryWait->pc88_binds, SIGNAL(sig_set_dipsw(int, bool)),
		 this, SLOT(set_dipsw(int, bool)));
   
#if defined(SUPPORT_PC88_OPNA)
   {
      int ii;
      menuSoundDevice = new QMenu(menuMachine);
      menuSoundDevice->setObjectName(QString::fromUtf8("menuSoundDevice_PC88"));
      menuMachine->addAction(menuSoundDevice->menuAction());
      
      actionGroup_SoundDevice = new QActionGroup(this);
      actionGroup_SoundDevice->setExclusive(true);
      for(ii = 0; ii < 2; ii++) {
	 actionSoundDevice[ii] = new Action_Control_88(this);
	 actionGroup_SoundDevice->addAction(actionSoundDevice[ii]);
	 actionSoundDevice[ii]->setCheckable(true);
	 actionSoundDevice[ii]->setVisible(true);
	 actionSoundDevice[ii]->pc88_binds->setValue1(ii);
	 if(config.sound_device_type == ii) actionSoundDevice[ii]->setChecked(true);
	 menuSoundDevice->addAction(actionSoundDevice[ii]);
	 connect(actionSoundDevice[ii], SIGNAL(triggered()),
		 actionSoundDevice[ii]->pc88_binds, SLOT(do_set_sound_device()));
	 connect(actionSoundDevice[ii]->pc88_binds, SIGNAL(sig_sound_device(int)),
		 this, SLOT(do_set_sound_device(int)));
      }
   }
#endif
#if defined(USE_DEVICE_TYPE)
   {
      int ii;
      menuDeviceType = new QMenu(menuMachine);
      menuDeviceType->setObjectName(QString::fromUtf8("menuDeviceType"));
      menuMachine->addAction(menuDeviceType->menuAction());
      
      actionGroup_DeviceType = new QActionGroup(this);
      actionGroup_DeviceType->setExclusive(true);
      for(ii = 0; ii < USE_DEVICE_TYPE; ii++) {
	 actionDeviceType[ii] = new Action_Control_88(this);
	 actionGroup_DeviceType->addAction(actionDeviceType[ii]);
	 actionDeviceType[ii]->setCheckable(true);
	 actionDeviceType[ii]->setVisible(true);
	 actionDeviceType[ii]->pc88_binds->setValue1(ii);
	 if(config.device_type == ii) actionDeviceType[ii]->setChecked(true);
	 menuDeviceType->addAction(actionDeviceType[ii]);
	 connect(actionDeviceType[ii], SIGNAL(triggered()),
		 actionDeviceType[ii]->pc88_binds, SLOT(do_set_device_type()));
	 connect(actionDeviceType[ii]->pc88_binds, SIGNAL(sig_device_type(int)),
		 this, SLOT(set_device_type(int)));
      }
   }
#endif
}


META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
   setupUI_Emu();
   retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



