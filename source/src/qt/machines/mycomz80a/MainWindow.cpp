/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for MycomZ80 .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QMenu>

#include "menuclasses.h"

#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE


void META_MainWindow::setupUI_Emu(void)
{
   menuMachine->setVisible(false);
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("",  false);
	retranslateOpMenuZ80(true);

#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif

//        menuRecord->setTitle(QApplication::translate("MainWindow", "Record", 0));
//        menuRecoad_as_movie->setTitle(QApplication::translate("MainWindow", "Recoad as movie", 0));

	// Set Labels
} // retranslateUi



META_MainWindow::META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE
