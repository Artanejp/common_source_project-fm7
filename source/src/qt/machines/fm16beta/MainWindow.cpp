/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for FM16β .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Mar 01, 2018 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */
#if defined(_USE_QT5)
#include <QVariant>
#else
#include <QtCore/QVariant>
#endif
#include <QtGui>
#include "menuclasses.h"
#include "commonclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE

void META_MainWindow::setupUI_Emu(void)
{
   int i;
   QString tmps;
}

void META_MainWindow::retranslateUi(void)
{
	int i;
	
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(" ",  false);
	
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
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



