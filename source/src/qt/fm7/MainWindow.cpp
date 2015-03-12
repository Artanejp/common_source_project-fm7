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
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE


Object_Menu_Control_7::Object_Menu_Control_7(QObject *parent) : Object_Menu_Control(parent)
{
}

Object_Menu_Control_7::~Object_Menu_Control_7()
{
}

void Object_Menu_Control_7::do_set_sound_device(void)
{
   emit sig_sound_device(this->getValue1());
}

void Object_Menu_Control_7::do_set_cyclesteal(bool flag)
{
   if(flag) {
	config.dipswitch = config.dipswitch | 0x0001;
   } else {
	config.dipswitch = config.dipswitch & ~0x0001;
   }
   if(emu) {
      emu->LockVM();
      emu->update_config();
      emu->UnlockVM();
   }

}

   
Action_Control_7::Action_Control_7(QObject *parent) : Action_Control(parent)
{
   fm7_binds = new Object_Menu_Control_7(parent);
   fm7_binds->setValue1(0);
}

Action_Control_7::~Action_Control_7()
{
   delete fm7_binds;
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

  retranslateControlMenu("HOT START", true);
  retranslateFloppyMenu(0, 1);
  retranslateFloppyMenu(1, 2);
  retranslateCMTMenu();
  retranslateSoundMenu();
  retranslateScreenMenu();
   
  this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
  
  
  actionCapture_Screen->setText(QApplication::translate("MainWindow", "Capture Screen", 0, QApplication::UnicodeUTF8));
  
  actionAbout->setText(QApplication::translate("MainWindow", "About...", 0, QApplication::UnicodeUTF8));
  

   //	actionStart_Record_Movie->setText(QApplication::translate("MainWindow", "Start Record Movie", 0, QApplication::UnicodeUTF8));
  //      actionStop_Record_Movie->setText(QApplication::translate("MainWindow", "Stop Record Movie", 0, QApplication::UnicodeUTF8));
  // 
  menuScreen->setTitle(QApplication::translate("MainWindow", "Screen", 0, QApplication::UnicodeUTF8));
  menuStretch_Mode->setTitle(QApplication::translate("MainWindow", "Stretch Mode", 0, QApplication::UnicodeUTF8));
  // FM-7 Specified
  menuCpuType->setTitle("CPU Frequency");
  actionCpuType[0]->setText(QString::fromUtf8("2MHz"));
  actionCpuType[1]->setText(QString::fromUtf8("1.2MHz"));


  menuBootMode->setTitle("BOOT Mode");
  actionBootMode[0]->setText(QString::fromUtf8("BASIC"));
  actionBootMode[1]->setText(QString::fromUtf8("DOS"));	
  actionBootMode[2]->setText(QString::fromUtf8("MMR"));

   actionCycleSteal->setText(QString::fromUtf8("Cycle Steal"));
   menuSoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Boards", 0, QApplication::UnicodeUTF8));
#if defined(_FM77AV_VARIANTS)
   actionSoundDevice[0]->setVisible(false);
   actionSoundDevice[2]->setVisible(false);
   actionSoundDevice[3]->setVisible(false);
   actionSoundDevice[6]->setVisible(false);
   actionSoundDevice[1]->setText(QString::fromUtf8("OPN"));
   actionSoundDevice[3]->setText(QString::fromUtf8("OPN+WHG"));
   actionSoundDevice[5]->setText(QString::fromUtf8("OPN+THG"));
   actionSoundDevice[7]->setText(QString::fromUtf8("OPN+WHG+THG"));
#else
   actionSoundDevice[0]->setText(QString::fromUtf8("PSG"));
   actionSoundDevice[1]->setText(QString::fromUtf8("PSG+OPN"));
   actionSoundDevice[2]->setText(QString::fromUtf8("PSG+WHG"));
   actionSoundDevice[3]->setText(QString::fromUtf8("PSG+OPN+WHG"));
   actionSoundDevice[4]->setText(QString::fromUtf8("PSG+THG"));
   actionSoundDevice[5]->setText(QString::fromUtf8("PSG+OPN+THG"));
   actionSoundDevice[6]->setText(QString::fromUtf8("PSG+WHG+THG"));
   actionSoundDevice[7]->setText(QString::fromUtf8("PSG+OPN+WHG+THG"));
#endif

// End.
// 
//        menuRecord->setTitle(QApplication::translate("MainWindow", "Record", 0, QApplication::UnicodeUTF8));
//        menuRecoad_as_movie->setTitle(QApplication::translate("MainWindow", "Recoad as movie", 0, QApplication::UnicodeUTF8));
	
  menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0, QApplication::UnicodeUTF8));
  menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0, QApplication::UnicodeUTF8));
  
  menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0, QApplication::UnicodeUTF8));
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

   ConfigCPUBootMode(3);
   
   {
      int ii;
      menuSoundDevice = new QMenu(menuMachine);
      menuSoundDevice->setObjectName(QString::fromUtf8("menuSoundDevice_FM7"));
      menuMachine->addAction(menuSoundDevice->menuAction());
      
      actionGroup_SoundDevice = new QActionGroup(this);
      actionGroup_SoundDevice->setExclusive(true);
      for(ii = 0; ii < 8; ii++) {
	 actionSoundDevice[ii] = new Action_Control_7(this);
	 actionGroup_SoundDevice->addAction(actionSoundDevice[ii]);
	 actionSoundDevice[ii]->setCheckable(true);
	 actionSoundDevice[ii]->setVisible(true);
	 actionSoundDevice[ii]->fm7_binds->setValue1(ii);
	 if(config.sound_device_type == ii) actionSoundDevice[ii]->setChecked(true);
	 menuSoundDevice->addAction(actionSoundDevice[ii]);
	 connect(actionSoundDevice[ii], SIGNAL(triggered()),
		 actionSoundDevice[ii]->fm7_binds, SLOT(do_set_sound_device()));
	 connect(actionSoundDevice[ii]->fm7_binds, SIGNAL(sig_sound_device(int)),
		 this, SLOT(do_set_sound_device(int)));
      }
   }
   
	actionCycleSteal = new Action_Control_7(this);
	menuMachine->addAction(actionCycleSteal);
	actionCycleSteal->setCheckable(true);
	actionCycleSteal->setVisible(true);
	if((config.dipswitch & 0x01) == 0x01) actionCycleSteal->setChecked(true);
	connect(actionCycleSteal, SIGNAL(toggled(bool)),
		 actionCycleSteal->fm7_binds, SLOT(do_set_cyclesteal(bool)));

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



