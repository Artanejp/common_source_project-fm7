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
#include "emu.h"
#include "commonclasses.h"
#include "menuclasses.h"

#include "qt_main.h"


void META_MainWindow::setupUI_Emu(void)
{
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("System Reset",  true);
	actionSpecial_Reset[0]->setText(QApplication::translate("MainWindow", "System Reset", 0));
	actionSpecial_Reset[0]->setToolTip(QApplication::translate("MainWindow", "Do system reset.", 0));
	retranslateOpMenuZ80(true);

#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
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
