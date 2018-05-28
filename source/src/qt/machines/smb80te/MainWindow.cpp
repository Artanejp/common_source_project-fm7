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
extern config_t config;

void META_MainWindow::do_set_adrs_8000(bool flag)
{
	if(flag) {
		config.dipswitch = config.dipswitch | 0x00000001;
	} else {
		config.dipswitch = config.dipswitch & ~0x00000001;
	}
}

void META_MainWindow::retranslateUi(void)
{

	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("", false);
	actionAddress8000->setText(QApplication::translate("MenuSMB80", "SW: ADRS SW 8000", 0));
	actionAddress8000->setToolTip(QApplication::translate("MenuSMB80", "SW: ADRS SW 8000", 0));
	menu_BINs[0]->setTitle(QApplication::translate("MenuSMB80", "RAM", 0));
	// Set Labels
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
	
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
	actionAddress8000 = new QAction(this);
	actionAddress8000->setCheckable(true);
	actionAddress8000->setVisible(true);
	menuMachine->addAction(actionAddress8000);
	actionAddress8000->setChecked(((config.dipswitch & 0x00000001) != 0) ? true : false);
	connect(actionAddress8000, SIGNAL(toggled(bool)), this, SLOT(do_set_adrs_8000(bool)));
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



