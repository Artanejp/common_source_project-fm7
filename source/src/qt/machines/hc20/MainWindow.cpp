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
	int i;
	QString tmps;
	menu_Emu_DipSw = new QMenu(menuMachine);
	menu_Emu_DipSw->setObjectName(QString::fromUtf8("menu_DipSw"));

	actionGroup_DipSw = new QActionGroup(this);
	actionGroup_DipSw->setExclusive(false);
	menuMachine->addAction(menu_Emu_DipSw->menuAction());
	for(i = 0; i < 4; i++) {
		SET_ACTION_SINGLE_DIPSWITCH_CONNECT(action_Emu_DipSw[i], (uint32_t)0x01 << i, p_config->dipswitch, SIGNAL(toggled(bool)), SLOT(do_set_single_dipswitch(bool)));
		
        tmps.number(i + 1);
        tmps = QString::fromUtf8("actionEmu_DipSw") + tmps;
        action_Emu_DipSw[i]->setObjectName(tmps);
		
		menu_Emu_DipSw->addAction(action_Emu_DipSw[i]);
		actionGroup_DipSw->addAction(action_Emu_DipSw[i]);
	}
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu(" ",  false);
   // Set Labels
	menu_Emu_DipSw->setTitle(QApplication::translate("Machine", "DIP Switches", 0));
	action_Emu_DipSw[0]->setText(QApplication::translate("Machine", "Dip Switch 1", 0));
	action_Emu_DipSw[1]->setText(QApplication::translate("Machine", "Dip Switch 2", 0));
	action_Emu_DipSw[2]->setText(QApplication::translate("Machine", "Dip Switch 3", 0));
	action_Emu_DipSw[3]->setText(QApplication::translate("MainWindow", "Dip Switch 4", 0));

#ifdef USE_DEBUGGER
	actionDebugger[1]->setText(QApplication::translate("MainWindow", "TF-20 CPU", 0));
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



