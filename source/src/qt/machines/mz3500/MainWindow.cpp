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
#include <QActionGroup>
#include <QMenu>

#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE

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

	uint32_t bit;
	const int bitpos[3] = {3, 7, 8};
	for(i = 0; i < 3; i++) {
		bit = 0x01 << bitpos[i];
		SET_ACTION_SINGLE_DIPSWITCH_CONNECT(action_Emu_DipSw[i], bit,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
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
	retranslateControlMenu("Halt",  true);
	retranslateOpMenuZ80(true);

	actionReset->setToolTip(QApplication::translate("MainWindow", "Do system reset.", 0));
	actionSpecial_Reset[0]->setText(QApplication::translate("MainWindow", "Halt", 0));
	actionSpecial_Reset[0]->setToolTip(QApplication::translate("MainWindow", "HALT a machine.", 0));

	this->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0));
	menu_Emu_DipSw->setTitle(QApplication::translate("MainWindow", "Dip Switches", 0));
	action_Emu_DipSw[0]->setText(QApplication::translate("MainWindow", "SW4: Period for Decimal Point", 0));
	action_Emu_DipSw[1]->setText(QApplication::translate("MainWindow", "FD1: Normally Capital Letter", 0));
	action_Emu_DipSw[2]->setText(QApplication::translate("MainWindow", "P/M: 3500 CG for 200 Line CRT", 0));

	actionPrintDevice[1]->setText(QApplication::translate("MainWindow", "MZ-1P17", 0));
	actionPrintDevice[1]->setToolTip(QApplication::translate("MainWindow", "Sharp MZ-1P17 kanji thermal printer.", 0));
#if defined(USE_MONITOR_TYPE)
	actionMonitorType[0]->setText(QApplication::translate("MachineMZ2500", "400Lines, Analog.", 0));
	actionMonitorType[1]->setText(QApplication::translate("MachineMZ2500", "400Lines, Digital.", 0));
	actionMonitorType[2]->setText(QApplication::translate("MachineMZ2500", "200Lines, Analog.", 0));
	actionMonitorType[3]->setText(QApplication::translate("MachineMZ2500", "200Lines, Digital.", 0));
#endif
#if defined(USE_DEBUGGER)
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(true);
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
