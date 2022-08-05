/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for X1TurboZ .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QVariant>
#include <QApplication>
#include <QtGui>
#include <QMenu>

#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(" ", false);
	QString tmps;
	QString n_tmps;
	int i;
	menu_Emu_DipSw->setTitle(QApplication::translate("MainWindow", "DIP Switches", 0));
	menu_Emu_DipSw->setToolTipsVisible(true);
	for(i = 0; i < 8; i++) {
		tmps = QApplication::translate("MainWindow", "DIP Switch #", 0);
		n_tmps.setNum(i + 1);
		action_Emu_DipSw[i]->setText(tmps + n_tmps);
	}
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
	int i;
	QString tmps;
	
	menuMachine->setVisible(true);
	menu_Emu_DipSw = new QMenu(menuMachine);
	menu_Emu_DipSw->setObjectName(QString::fromUtf8("menu_DipSw"));
	actionGroup_DipSw = new QActionGroup(this);
	actionGroup_DipSw->setExclusive(false);
	menuMachine->addAction(menu_Emu_DipSw->menuAction());
	
	for(i = 0; i < 8; i++) {
		SET_ACTION_SINGLE_DIPSWITCH_CONNECT(action_Emu_DipSw[i], (0x01 << i), p_config->dipswitch , SIGNAL(toggled(bool)) , SLOT(do_set_single_dipswitch(bool)));
		
		tmps.number(i + 1);
		tmps = QString::fromUtf8("actionEmu_DipSw") + tmps;
		action_Emu_DipSw[i]->setObjectName(tmps);
		menu_Emu_DipSw->addAction(action_Emu_DipSw[i]);
		actionGroup_DipSw->addAction(action_Emu_DipSw[i]);
	}

}


META_MainWindow::META_MainWindow(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}


