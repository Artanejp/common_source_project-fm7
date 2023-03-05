/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for COMPUTER RESEARCH CRC-80.
 * (C) 2023 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 17, 2023 : Split from eM23
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

extern config_t config;
//QT_BEGIN_NAMESPACE

void META_MainWindow::retranslateUi(void)
{
	Ui_MainWindowBase::retranslateUi();
	retranslateControlMenu("", false);
	retranslateOpMenuZ80(true);

#if defined(USE_DIPSWITCH)
	action_DipSWs[0]->setText(QApplication::translate("MachineCRC80", "SW1: STEP", 0));
	action_DipSWs[1]->setText(QApplication::translate("MachineCRC80", "SW2: TTY", 0));
	for(int i = 0; i < 2; i++) {
		action_DipSWs[i]->setVisible(true);
	}
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
	for(int i = 0; i < 2; i++) {
		action_DipSWs[i] = nullptr;
	}
	menuMachine->addSeparator();
	uint32_t _bit = 0x00000001;
	for(int i = 0; i < 2; i++) {
		SET_ACTION_SINGLE_DIPSWITCH_CONNECT(action_DipSWs[i], _bit,
										p_config->dipswitch,
										SIGNAL(toggled(bool)),
										SLOT(do_set_single_dipswitch(bool)));
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
