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
	config.sound_device_type = num;
	this->do_emu_update_config();
#endif
}

void META_MainWindow::retranslateUi(void)
{
	const char *title="";
	retranslateControlMenu(title, false);
	retranslateFloppyMenu(0, 1);
	retranslateFloppyMenu(1, 2);
#if defined(USE_FD3)
	retranslateFloppyMenu(2, 3);
#endif   
#if defined(USE_FD4)
	retranslateFloppyMenu(3, 4);
#endif   
	retranslateCMTMenu();
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
   
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
  
#ifdef USE_DEBUGGER
	actionDebugger_1->setText(QApplication::translate("MainWindow", "Main CPU", 0));
	actionDebugger_2->setText(QApplication::translate("MainWindow", "Sub  CPU", 0));
	actionDebugger_3->setText(QApplication::translate("MainWindow", "PC-80S31K", 0));
#endif	

   // Set Labels
  
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{

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



