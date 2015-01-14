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

void META_MainWindow::do_set_sound_device(int num)
{
   if((num < 0) || (num >= 3)) return;
#ifdef USE_SOUND_DEVICE_TYPE
   if(emu) {
      config.sound_device_type = num;
      emu->LockVM();
      emu->update_config();
      emu->UnlockVM();
   }
#endif
}


	

void META_MainWindow::setupUI_Emu(void)
{
   int i;
   menu_Emu_SoundDevice = new QMenu(menuEmulator);
   menu_Emu_SoundDevice->setObjectName(QString::fromUtf8("menu_SoundDevice"));
   
   actionGroup_SoundDevice = new QActionGroup(this);
   actionGroup_SoundDevice->setObjectName(QString::fromUtf8("actionGroup_SoundDevice"));
   actionGroup_SoundDevice->setExclusive(true);
   menuEmulator->addAction(menu_Emu_SoundDevice->menuAction());   
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
	      this, SLOT(do_set_sound_device(int)));
   }
   // Set Labels
   menu_Emu_SoundDevice->setTitle(QApplication::translate("MainWindow", "Sound Device", 0, QApplication::UnicodeUTF8));
   action_Emu_SoundDevice[0]->setText(QApplication::translate("MainWindow", "PSG", 0, QApplication::UnicodeUTF8));
   action_Emu_SoundDevice[1]->setText(QApplication::translate("MainWindow", "CZ-8BS1 Single", 0, QApplication::UnicodeUTF8));
   action_Emu_SoundDevice[2]->setText(QApplication::translate("MainWindow", "CZ-8BS1 Double", 0, QApplication::UnicodeUTF8));
}
					    

META_MainWindow::META_MainWindow(QWidget *parent) : Ui_MainWindow(parent)
{
   setupUI_Emu();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



