/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QMenu>
#include <QActionGroup>

#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE

extern config_t config;

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
#ifdef USE_SOUND_TYPE
	config.sound_type = num;
	this->do_emu_update_config();
#endif
}

void META_MainWindow::retranslateUi(void)
{
	const char *title="";
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(title, false);
#ifdef USE_DEBUGGER
	actionDebugger[0]->setText(QApplication::translate("MenuPC6001", "Main CPU", 0));
	actionDebugger[1]->setText(QApplication::translate("MenuPC6001", "Sub  CPU", 0));
	actionDebugger[2]->setText(QApplication::translate("MenuPC6001", "PC-80S31K", 0));
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
	actionDebugger[2]->setVisible(true);
	actionDebugger[3]->setVisible(false);
#endif	
#if defined(USE_PRINTER)
	actionPrintDevice[1]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MenuPC6001", "NEC PC-PR201 kanji serial printer.", 0));
	actionPrintDevice[1]->setEnabled(false);
#endif

   // Set Labels
  
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{

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



