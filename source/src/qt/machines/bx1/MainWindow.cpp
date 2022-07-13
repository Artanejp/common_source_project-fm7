/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for Super Casette Vision .
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2015 : Initial, many of constructors were moved to qt/gui/menu_main.cpp.
 */

#include <QApplication>
#include <QVariant>
#include <QtGui>
#include <QMenu>

#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"

//QT_BEGIN_NAMESPACE
	
void META_MainWindow::setupUI_Emu(void)
{
	menuMachine->addSeparator();
	uint32_t _bit = 0x00000001;
	for(int i = 0; i < 4; i++) {
		SET_ACTION_SINGLE_DIPSWITCH_CONNECT(action_DipSWs[i], _bit,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
		menuMachine->addAction(action_DipSWs[i]);
		_bit <<= 1;
	}
}

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("",  false);
	action_DipSWs[0]->setText(QApplication::translate("MachineBX1", "AUTO PRINT Switch", 0));
	action_DipSWs[1]->setText(QApplication::translate("MachineBX1", "PROG.SELECT Switch", 0));
	action_DipSWs[2]->setText(QApplication::translate("MachineBX1", "JPN/ENG Jumper", 0));
	action_DipSWs[0]->setVisible(true);
	action_DipSWs[1]->setVisible(true);
	action_DipSWs[2]->setVisible(true);
	action_DipSWs[3]->setVisible(false);

	actionPrintDevice[3]->setText(QApplication::translate("MachineBX1", "None", 0));
	
	actionPrintDevice[0]->setVisible(true);
	actionPrintDevice[1]->setVisible(false);
	actionPrintDevice[2]->setVisible(false);
	actionPrintDevice[3]->setVisible(true);


	actionPrintDevice[0]->setEnabled(true);
	actionPrintDevice[1]->setEnabled(false);
	actionPrintDevice[1]->setEnabled(false);
	actionPrintDevice[3]->setEnabled(true);

#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
} // retranslateUi



META_MainWindow::META_MainWindow(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent) : Ui_MainWindow(p, logger, parent)
{
	for(int i = 0; i < 4; i++) {
		action_DipSWs[i] = NULL;
	}
	setupUI_Emu();
	retranslateUi();
}


META_MainWindow::~META_MainWindow()
{
}

//QT_END_NAMESPACE



