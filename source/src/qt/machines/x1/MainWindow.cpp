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
#include "menu_cart.h"
#include "emu.h"
#include "qt_main.h"

Action_Control_X1::Action_Control_X1(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	x1_binds = new Object_Menu_Control_X1(parent, p);
}

Action_Control_X1::~Action_Control_X1(){
	delete x1_binds;
}

Object_Menu_Control_X1::Object_Menu_Control_X1(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_X1::~Object_Menu_Control_X1(){
}

#if !defined(_X1)
void Object_Menu_Control_X1::do_set_display_mode(void)
{
	emit sig_display_mode(getValue1());
}
#endif

extern config_t config;

void META_MainWindow::setupUI_Emu(void)
{
   int i;
# if !defined(_X1)
   menu_Emu_DisplayMode = new QMenu(menuMachine);
   menu_Emu_DisplayMode->setObjectName(QString::fromUtf8("menu_DisplayMode"));
   
   actionGroup_DisplayMode = new QActionGroup(this);
   actionGroup_DisplayMode->setObjectName(QString::fromUtf8("actionGroup_DisplayMode"));
   actionGroup_DisplayMode->setExclusive(true);
   menuMachine->addAction(menu_Emu_DisplayMode->menuAction());   
   for(i = 0; i < 2; i++) {
	   action_Emu_DisplayMode[i] = new Action_Control_X1(this, using_flags);
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
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("NMI Reset",  true);
	retranslateFloppyMenu(0, 0, QApplication::translate("MachineX1", "FDD", 0));
	retranslateFloppyMenu(1, 1, QApplication::translate("MachineX1", "FDD", 0));

	actionSpecial_Reset->setText(QApplication::translate("Machine", "NMI Reset", 0));
	actionSpecial_Reset->setToolTip(QApplication::translate("MachineX1", "Do NMI reset.", 0));

	// Set Labels
	menuSoundDevice->setTitle(QApplication::translate("MachineX1", "Sound Device", 0));
	actionSoundDevice[0]->setText(QApplication::translate("MachineX1", "PSG", 0));
	actionSoundDevice[1]->setText(QApplication::translate("MachineX1", "CZ-8BS1 Single", 0));
	actionSoundDevice[2]->setText(QApplication::translate("MachineX1", "CZ-8BS1 Double", 0));
	
#if !defined(_X1)
	menu_Emu_DisplayMode->setTitle(QApplication::translate("MachineX1", "Display Mode", 0));
	action_Emu_DisplayMode[0]->setText(QApplication::translate("MachineX1", "High Resolution (400line)", 0));
	action_Emu_DisplayMode[1]->setText(QApplication::translate("MachineX1", "Standarsd Resolution (200line)", 0));
#else
	menu_Emu_DisplayMode->setVisible(false);
#endif
#if defined(USE_KEYBOARD_TYPE)
	menuKeyboardType->setTitle(QApplication::translate("MachineX1", "Keyboard Mode", 0));
	actionKeyboardType[0]->setText(QApplication::translate("MachineX1", "Mode A", 0));
	actionKeyboardType[1]->setText(QApplication::translate("MachineX1", "Mode B", 0));
#endif
#if defined(USE_JOYSTICK_TYPE)
	menuJoystickType->setTitle(QApplication::translate("MachineX1", "Joy PAD Type", 0));
	actionJoystickType[0]->setText(QApplication::translate("MachineX1", "2-Buttons Joy Pad", 0));
	actionJoystickType[1]->setText(QApplication::translate("MachineX1", "6-Buttons Joy Pad", 0));
	actionJoystickType[2]->setText(QApplication::translate("MachineX1", "2-Buttons with Multi-Tap", 0));
	actionJoystickType[3]->setText(QApplication::translate("MachineX1", "6-Buttons with Multi-Tap", 0));
#endif
#if defined(USE_DRIVE_TYPE)
	menuDriveType->setTitle(QApplication::translate("MachineX1", "Floppy Type", 0));
	actionDriveType[0]->setText(QApplication::translate("MachineX1", "2D", 0));
	actionDriveType[1]->setText(QApplication::translate("MachineX1", "2DD", 0));
	actionDriveType[2]->setText(QApplication::translate("MachineX1", "2HD", 0));
#endif
#ifdef USE_DEBUGGER
	actionDebugger[3]->setVisible(false);

	actionDebugger[0]->setText(QApplication::translate("MachineX1", "Main CPU", 0));
	actionDebugger[1]->setText(QApplication::translate("MachineX1", "Sub CPU", 0));
	actionDebugger[2]->setText(QApplication::translate("MachineX1", "Keyboard CPU", 0));
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
	actionDebugger[2]->setVisible(true);
#ifdef _X1TWIN	
	actionDebugger[3]->setText(QApplication::translate("MachineX1", "PC-ENGINE CPU", 0));
	actionDebugger[3]->setVisible(true);
#endif
#endif	
#if defined(USE_PRINTER)
	actionPrintDevice[1]->setText(QString::fromUtf8("Sharp MZ-1P17"));
	actionPrintDevice[2]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MachineX1", "Sharp MZ-1P17 kanji thermal printer.", 0));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MachineX1", "NEC PC-PR201 kanji serial printer.", 0));
	actionPrintDevice[2]->setEnabled(false);
#endif
#if defined(_X1TWIN)
	menu_Cart[0]->setTitle(QApplication::translate("MachineX1", "HuCARD", 0));
#endif	
	
} // retranslateUi



META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



