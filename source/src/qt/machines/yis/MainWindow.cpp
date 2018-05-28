/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for Yamaha YIS .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QActionGroup>
#include <QMenu>

#include "menuclasses.h"
#include "commonclasses.h"

#include "emu.h"
#include "qt_main.h"
#include "vm.h"

//QT_BEGIN_NAMESPACE

extern config_t config;

Object_Menu_Control_YIS::Object_Menu_Control_YIS(QObject *parent, USING_FLAGS *p) : Object_Menu_Control(parent, p)
{
}

Object_Menu_Control_YIS::~Object_Menu_Control_YIS()
{
}

void Object_Menu_Control_YIS::do_set_display_mode(void)
{
	emit sig_display_mode(getValue1());
}

Action_Control_YIS::Action_Control_YIS(QObject *parent, USING_FLAGS *p) : Action_Control(parent, p)
{
	yis_binds = new Object_Menu_Control_YIS(parent, p);
    yis_binds->setValue1(0);
}

Action_Control_YIS::~Action_Control_YIS()
{
	delete yis_binds;
}


void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("", false);
   // Set Labels
#ifdef USE_MONITOR_TYPE
	menu_Emu_DisplayMode->setTitle(QApplication::translate("MainWindow", "Display Mode", 0));
	action_Emu_DisplayMode[0]->setText(QApplication::translate("MainWindow", "PU-1-10", 0));
	action_Emu_DisplayMode[1]->setText(QApplication::translate("MainWindow", "PU-1-20", 0));
	action_Emu_DisplayMode[2]->setText(QApplication::translate("MainWindow", "PU-10", 0));
#endif	
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
  
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
#ifdef USE_MONITOR_TYPE
   menu_Emu_DisplayMode = new QMenu(menuMachine);
   menu_Emu_DisplayMode->setObjectName(QString::fromUtf8("menu_DisplayMode"));
   
   actionGroup_DisplayMode = new QActionGroup(this);
   actionGroup_DisplayMode->setObjectName(QString::fromUtf8("actionGroup_DisplayMode"));
   actionGroup_DisplayMode->setExclusive(true);
   menuMachine->addAction(menu_Emu_DisplayMode->menuAction());   
   for(int i = 0; i < USE_MONITOR_TYPE; i++) {
	   action_Emu_DisplayMode[i] = new Action_Control_YIS(this, using_flags);
	   action_Emu_DisplayMode[i]->setCheckable(true);
	   action_Emu_DisplayMode[i]->yis_binds->setValue1(i);
	   if(i == config.monitor_type) action_Emu_DisplayMode[i]->setChecked(true); // Need to write configure
   }
   action_Emu_DisplayMode[0]->setObjectName(QString::fromUtf8("action_Emu_Display_PU_1_10"));
   action_Emu_DisplayMode[1]->setObjectName(QString::fromUtf8("action_Emu_Display_PU_1_20"));
   action_Emu_DisplayMode[2]->setObjectName(QString::fromUtf8("action_Emu_Display_PU_10"));
   for(int i = 0; i < USE_MONITOR_TYPE; i++) {
	   menu_Emu_DisplayMode->addAction(action_Emu_DisplayMode[i]);
	   actionGroup_DisplayMode->addAction(action_Emu_DisplayMode[i]);
	   connect(action_Emu_DisplayMode[i], SIGNAL(triggered()),
			   action_Emu_DisplayMode[i]->yis_binds, SLOT(do_set_display_mode()));
	   connect(action_Emu_DisplayMode[i]->yis_binds, SIGNAL(sig_display_mode(int)),
			   this, SLOT(set_monitor_type(int)));
   }
#endif
}


META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



