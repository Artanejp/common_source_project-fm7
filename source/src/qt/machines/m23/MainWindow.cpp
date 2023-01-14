/*
 * Common Source code Project:
 * Ui->Qt->MainWindow for SORE M23 / M68.
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *   License : GPLv2
 *   History :
 * Jan 14, 2022 : Split from eMP85
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

#if defined(USE_DIPSWITCH)
	action_DipSWs[0]->setText(QApplication::translate("MachineM23", "SW1: OS Boot/Debug", 0));
	action_DipSWs[1]->setText(QApplication::translate("MachineM23", "SW2: Debug", 0));
	action_DipSWs[2]->setText(QApplication::translate("MachineM23", "SW3: Debug", 0));
	action_DipSWs[3]->setText(QApplication::translate("MachineM23", "SW4: (Reserved)", 0));
	action_DipSWs[4]->setText(QApplication::translate("MachineM23", "SW5: (Reserved)", 0));
	action_DipSWs[5]->setText(QApplication::translate("MachineM23", "SW6: (Reserved)", 0));
	action_DipSWs[6]->setText(QApplication::translate("MachineM23", "SW7: (Reserved)", 0));
	action_DipSWs[7]->setText(QApplication::translate("MachineM23", "SW8: Memory Page #0", 0));
	for(int i = 0; i < 8; i++) {
		action_DipSWs[i]->setVisible(true);
	}
#endif	
#if defined(USE_MONITOR_TYPE)
	actionMonitorType[0]->setText(QApplication::translate("MenuM23", "Color Monitor"));
	actionMonitorType[1]->setText(QApplication::translate("MenuM23", "Green Monitor"));
#endif	
	
#if defined(USE_DRIVE_TYPE)
	menuDriveType->setTitle(QApplication::translate("MachineM23", "Floppy Type", 0));
	actionDriveType[0]->setText(QApplication::translate("MachineM23", "FD20-I (3.5inch-1DD)", 0));
	actionDriveType[1]->setText(QApplication::translate("MachineM23", "FD20 (5.25inch-1DD)", 0));
	actionDriveType[2]->setText(QApplication::translate("MachineM23", "DF44H (8inch-2D)", 0));
#endif

#if defined(USE_KEYBOARD_TYPE)
	menuKeyboardType->setTitle(QApplication::translate("MachineM23", "Keyboard Mode", 0));
	actionKeyboardType[0]->setText(QApplication::translate("MachineM23", "ASCII Keyboard", 0));
	actionKeyboardType[1]->setText(QApplication::translate("MachineM23", "JIS Keyboard", 0));
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
	for(int i = 0; i < 8; i++) {
		action_DipSWs[i] = nullptr;
	}
	menuMachine->addSeparator();
	uint32_t _bit = 0x00000001;
	for(int i = 0; i < 8; i++) {
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



