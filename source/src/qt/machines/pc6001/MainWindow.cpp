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


Object_Menu_Control_60::Object_Menu_Control_60(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_60::~Object_Menu_Control_60()
{
}

void Object_Menu_Control_60::do_set_sound_device(void)
{
	emit sig_sound_device(this->getValue1());
}


Action_Control_60::Action_Control_60(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	pc60_binds = new Object_Menu_Control_60(parent, p);
	pc60_binds->setValue1(0);
}

Action_Control_60::~Action_Control_60()
{
	delete pc60_binds;
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
#if defined(USE_CART1)
	retranslateCartMenu(0, 1);
#endif   
	retranslateCMTMenu();
	retranslateSoundMenu();
	retranslateScreenMenu();
	retranslateMachineMenu();
	retranslateEmulatorMenu();
	retranslateUI_Help();
   
	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
  
#ifdef USE_DEBUGGER
	actionDebugger[0]->setText(QApplication::translate("MainWindow", "Main CPU", 0));
	actionDebugger[1]->setText(QApplication::translate("MainWindow", "Sub  CPU", 0));
	actionDebugger[2]->setText(QApplication::translate("MainWindow", "PC-80S31K", 0));
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
	actionDebugger[2]->setVisible(true);
	actionDebugger[3]->setVisible(false);
#endif	
#if defined(USE_PRINTER)
	//actionPrintDevice[1]->setText(QString::fromUtf8("PC-PR201"));
#endif

   // Set Labels
  
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{

}


META_MainWindow::META_MainWindow(USING_FLAGS *p, QWidget *parent) : Ui_MainWindow(p, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



