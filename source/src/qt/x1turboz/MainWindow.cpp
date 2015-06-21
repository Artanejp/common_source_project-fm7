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


Action_Control_X1::Action_Control_X1(QObject *parent) : Action_Control(parent)
{
   x1_binds = new Object_Menu_Control_X1(parent);
}

Action_Control_X1::~Action_Control_X1(){
   delete x1_binds;
}

Object_Menu_Control_X1::Object_Menu_Control_X1(QObject *parent) : Object_Menu_Control(parent)
{
}

Object_Menu_Control_X1::~Object_Menu_Control_X1(){
}

void Object_Menu_Control_X1::do_set_sound_device(void)
{
   emit sig_sound_device(getValue1());
}

#ifdef _X1TURBOZ
void Object_Menu_Control_X1::do_set_display_mode(void)
{
   emit sig_display_mode(getValue1());
}
#endif

#ifdef USE_DEVICE_TYPE
void Object_Menu_Control_X1::do_set_device_type(void)
{
   emit sig_device_type(getValue1());
}
#endif
#ifdef USE_DRIVE_TYPE
void Object_Menu_Control_X1::do_set_drive_type(void)
{
   emit sig_drive_type(getValue1());
}
#endif
	

void META_MainWindow::setupUI_Emu(void)
{
   int i;
   menu_Emu_SoundDevice = new QMenu(menuMachine);
   menu_Emu_SoundDevice->setObjectName(QString::fromUtf8("menu_SoundDevice"));
   
   actionGroup_SoundDevice = new QActionGroup(this);
   actionGroup_SoundDevice->setObjectName(QString::fromUtf8("actionGroup_SoundDevice"));
   actionGroup_SoundDevice->setExclusive(true);
   menuMachine->addAction(menu_Emu_SoundDevice->menuAction());   
   for(i = 0; i < 3; i++) {
	action_Emu_SoundDevice[i] = new Action_Control_X1(this);
        action_Emu_SoundDevice[i]->setCheckable(true);
        action_Emu_SoundDevice[i]->x1_binds->setValue1(i);
        if(i == 2) action_Emu_SoundDevice[i]->setChecked(true); // Need to write configure
   }
   
   action_Emu_SoundDevice[0]->setObjectName(QString::fromUtf8("action_Emu_SoundDevice_PSG"));
   action_Emu_SoundDevice[1]->setObjectName(QString::fromUtf8("action_Emu_SoundDevice_FM1"));
   action_Emu_SoundDevice[2]->setObjectName(QString::fromUtf8("action_Emu_SoundDevice_FM2"));
   for(i = 0; i < 3; i++) {
      menu_Emu_SoundDevice->addAction(action_Emu_SoundDevice[i]);
      actionGroup_SoundDevice->addAction(action_Emu_SoundDevice[i]);
      connect(action_Emu_SoundDevice[i], SIGNAL(triggered()),
	      action_Emu_SoundDevice[i]->x1_binds, SLOT(do_set_sound_device()));
      connect(action_Emu_SoundDevice[i]->x1_binds, SIGNAL(sig_sound_device(int)),
	      this, SLOT(set_sound_device(int)));
   }
#ifdef USE_DEVICE_TYPE
   menuDeviceType = new QMenu(menuMachine);
   menuDeviceType->setObjectName(QString::fromUtf8("menu_DeviceType"));
   
   actionGroup_DeviceType = new QActionGroup(this);
   actionGroup_DeviceType->setObjectName(QString::fromUtf8("actionGroup_DeviceType"));
   actionGroup_DeviceType->setExclusive(true);
   menuMachine->addAction(menuDeviceType->menuAction());   
   for(i = 0; i < USE_DEVICE_TYPE; i++) {
      actionDeviceType[i] = new Action_Control_X1(this);
      actionDeviceType[i]->setCheckable(true);
      actionDeviceType[i]->setVisible(true);
      actionDeviceType[i]->x1_binds->setValue1(i);
      actionGroup_DeviceType->addAction(actionDeviceType[i]);
      menuDeviceType->addAction(actionDeviceType[i]);
      if(i == config.device_type) actionDeviceType[i]->setChecked(true); // Need to write configure
      connect(actionDeviceType[i], SIGNAL(triggered()),
	      actionDeviceType[i]->x1_binds, SLOT(do_set_device_type()));
      connect(actionDeviceType[i]->x1_binds, SIGNAL(sig_device_type(int)),
	      this, SLOT(set_device_type(int)));
   }
#endif
#ifdef USE_DRIVE_TYPE
   menuDriveType = new QMenu(menuMachine);
   menuDriveType->setObjectName(QString::fromUtf8("menu_DriveType"));
   
   actionGroup_DriveType = new QActionGroup(this);
   actionGroup_DriveType->setObjectName(QString::fromUtf8("actionGroup_DriveType"));
   actionGroup_DriveType->setExclusive(true);
   menuMachine->addAction(menuDriveType->menuAction());
   for(i = 0; i < USE_DRIVE_TYPE; i++) {
      actionDriveType[i] = new Action_Control_X1(this);
      actionDriveType[i]->setCheckable(true);
      actionDriveType[i]->setVisible(true);
      actionDriveType[i]->x1_binds->setValue1(i);
      if(i == config.drive_type) actionDriveType[i]->setChecked(true); // Need to write configure
      actionGroup_DriveType->addAction(actionDriveType[i]);
      menuDriveType->addAction(actionDriveType[i]);
      connect(actionDriveType[i], SIGNAL(triggered()),
	      actionDriveType[i]->x1_binds, SLOT(do_set_drive_type()));
      connect(actionDriveType[i]->x1_binds, SIGNAL(sig_drive_type(int)),
	      this, SLOT(set_drive_type(int)));
   }
#endif
   
# if defined(_X1TURBOZ)
   menu_Emu_DisplayMode = new QMenu(menuMachine);
   menu_Emu_DisplayMode->setObjectName(QString::fromUtf8("menu_DisplayMode"));
   
   actionGroup_DisplayMode = new QActionGroup(this);
   actionGroup_DisplayMode->setObjectName(QString::fromUtf8("actionGroup_DisplayMode"));
   actionGroup_DisplayMode->setExclusive(true);
   menuMachine->addAction(menu_Emu_DisplayMode->menuAction());   
   for(i = 0; i < 2; i++) {
      action_Emu_DisplayMode[i] = new Action_Control_X1(this);
      action_Emu_DisplayMode[i]->setCheckable(true);
      action_Emu_DisplayMode[i]->x1_binds->setValue1(i);
      if(i == config.monitor_type) action_Emu_DisplayMode[i]->setChecked(true); // Need to write configure
   }
   
   action_Emu_DisplayMode[0]->setObjectName(QString::fromUtf8("action_Emu_DisplayMode_High"));
   action_Emu_DisplayMode[1]->setObjectName(QString::fromUtf8("action_Emu_DisplayMode_Standard"));
   for(i = 0; i < 2; i++) {
      menu_Emu_DisplayMode->addAction(action_Emu_DisplayMode[i]);
      actionGroup_DisplayMode->addAction(action_Emu_DisplayMode[i]);
      connect(action_Emu_DisplayMode[i], SIGNAL(triggered()),
	      action_Emu_DisplayMode[i]->x1_binds, SLOT(do_set_display_mode()));
      connect(action_Emu_DisplayMode[i]->x1_binds, SIGNAL(sig_display_mode(int)),
	      this, SLOT(set_monitor_type(int)));
   }
#endif

}

void META_MainWindow::retranslateUi(void)
{
  retranslateControlMenu("NMI Reset",  true);
  retranslateFloppyMenu(0, 0);
  retranslateFloppyMenu(1, 1);
  retranslateCMTMenu();
  retranslateSoundMenu();
  retranslateScreenMenu();
   
  this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
  
  
  actionAbout->setText(QApplication::translate("MainWindow", "About...", 0, QApplication::UnicodeUTF8));
  

  menuEmulator->setTitle(QApplication::translate("MainWindow", "Emulator", 0, QApplication::UnicodeUTF8));
  menuMachine->setTitle(QApplication::translate("MainWindow", "Machine", 0, QApplication::UnicodeUTF8));
  
  menuHELP->setTitle(QApplication::translate("MainWindow", "HELP", 0, QApplication::UnicodeUTF8));
   // Set Labels
  menu_Emu_SoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Device", 0, QApplication::UnicodeUTF8));
  action_Emu_SoundDevice[0]->setText(QApplication::translate("MainWindow", "PSG", 0, QApplication::UnicodeUTF8));
  action_Emu_SoundDevice[1]->setText(QApplication::translate("MainWindow", "CZ-8BS1 Single", 0, QApplication::UnicodeUTF8));
  action_Emu_SoundDevice[2]->setText(QApplication::translate("MainWindow", "CZ-8BS1 Double", 0, QApplication::UnicodeUTF8));

#if defined(_X1TURBOZ)
  menu_Emu_DisplayMode->setTitle(QApplication::translate("MainWindow", "Display Mode", 0, QApplication::UnicodeUTF8));
  action_Emu_DisplayMode[0]->setText(QApplication::translate("MainWindow", "High Resolution (400line)", 0, QApplication::UnicodeUTF8));
  action_Emu_DisplayMode[1]->setText(QApplication::translate("MainWindow", "Standarsd Resolution (200line)", 0, QApplication::UnicodeUTF8));
#endif
#if defined(USE_DEVICE_TYPE)
  menuDeviceType->setTitle(QApplication::translate("MainWindow", "Keyboard Mode", 0, QApplication::UnicodeUTF8));
  actionDeviceType[0]->setText(QApplication::translate("MainWindow", "Mode A", 0, QApplication::UnicodeUTF8));
  actionDeviceType[1]->setText(QApplication::translate("MainWindow", "Mode B", 0, QApplication::UnicodeUTF8));
#endif
#if defined(USE_DRIVE_TYPE)
  menuDriveType->setTitle(QApplication::translate("MainWindow", "Floppy Type", 0, QApplication::UnicodeUTF8));
  actionDriveType[0]->setText(QApplication::translate("MainWindow", "2D", 0, QApplication::UnicodeUTF8));
  actionDriveType[1]->setText(QApplication::translate("MainWindow", "2HD", 0, QApplication::UnicodeUTF8));
#endif
#ifdef USE_DEBUGGER
	actionDebugger_1->setText(QApplication::translate("MainWindow", "Main CPU", 0, QApplication::UnicodeUTF8));
	actionDebugger_2->setText(QApplication::translate("MainWindow", "Sub CPU", 0, QApplication::UnicodeUTF8));
	actionDebugger_3->setText(QApplication::translate("MainWindow", "Keyboard CPU", 0, QApplication::UnicodeUTF8));
#endif	
   
} // retranslateUi



META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
   setupUI_Emu();
   retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



