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

#include "config.h"
#include "commonclasses.h"
#include "menuclasses.h"
#include "emu.h"
#include "qt_main.h"
#include "menu_cmt.h"
#include "menu_binary.h"

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("", false);
	// Will implement : RAM LOAD/SAVE

	if(menu_BINs[0] != NULL) menu_BINs[0]->setTitle(QApplication::translate("MachineTK80BS", "RAM", 0));
#if defined(_TK80BS)	
	if(menu_CMT[1] != NULL) menu_CMT[1]->setTitle(QApplication::translate("MachineTK80BS", "BS-CMT", 0));
#endif
	action_DipSWs[0]->setText(QApplication::translate("MachineTK80BS", "STEP/AUTO", 0));
	action_DipSWs[0]->setToolTip(QApplication::translate("MachineTK80BS", "If enabled, interrupt per instruction.\nUseful for debugging.", 0));
	action_DipSWs[0]->setVisible(true);
	
	menuBootMode->setTitle(QApplication::translate("MachineTK80BS", "BOOT Mode", 0));
	menuBootMode->setVisible(true);
#if defined(_TK80BS)
	actionBootMode[0]->setText(QString::fromUtf8("L1 BASIC"));
	actionBootMode[1]->setText(QString::fromUtf8("L2 BASIC"));	
	actionBootMode[0]->setToolTip(QApplication::translate("MachineTK80BS", "Use L1 BASIC.", 0));
	actionBootMode[1]->setToolTip(QApplication::translate("MachineTK80BS", "Use L2 BASIC.", 0));
#endif
	
#ifdef USE_DEBUGGER
	actionDebugger[0]->setVisible(true);
	actionDebugger[1]->setVisible(false);
	actionDebugger[2]->setVisible(false);
	actionDebugger[3]->setVisible(false);
#endif
	
} // retranslateUi

void META_MainWindow::setupUI_Emu(void)
{
	ConfigCPUBootMode(2);
	for(int i = 0; i < 4; i++) {
		action_DipSWs[i] = NULL;
	}
	menuMachine->addSeparator();
	uint32_t _bit = 0x00000001;
	for(int i = 0; i < 1; i++) {
		SET_ACTION_SINGLE_DIPSWITCH_CONNECT(action_DipSWs[i], _bit, p_config->dipswitch , SIGNAL(toggled(bool)) , SLOT(do_set_single_dipswitdh(bool)));
		menuMachine->addAction(action_DipSWs[i]);
		_bit <<= 1;
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

//QT_END_NAMESPACE



