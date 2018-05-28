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
#include "commonclasses.h"
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
	retranslateControlMenu("Reset",  true);
	actionReset->setText(QApplication::translate("MainWindow", "IPL Reset", 0));
	actionReset->setToolTip(QApplication::translate("MainWindow", "Do IPL reset.", 0));
	//actionSpecial_Reset->setToolTip(QApplication::translate("MainWindow", "Do system reset.", 0));
#if defined(USE_PRINTER)
	actionPrintDevice[1]->setText(QString::fromUtf8("MZ-1P17"));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MainWindow", "Sharp MZ-1P17 kanji thermal printer.", 0));
	actionPrintDevice[2]->setText(QString::fromUtf8("PC-PR201"));
	actionPrintDevice[2]->setToolTip(QApplication::translate("MainWindow", "NEC PC-PR201 kanji printer.", 0));
	actionPrintDevice[2]->setEnabled(false);
#endif
#if defined(USE_DEBUGGER)
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
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



