/*
 * Common Source Code Project for Qt : movie saver.
 * (C) 2016 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 *  License: GPLv2
 *  History: April 07, 2016 : Initial.
 */

#include <QApplication>
#include <QClipboard>
#include "qt_emuevents.h"
#include "qt_main.h"
#include "qt_dialogs.h"
#include "csp_logger.h"

extern EMU *emu;

void Ui_MainWindowBase::OnReset(void)
{
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GUI, "Reset");
	emit sig_vm_reset();
}

void Ui_MainWindowBase::OnSpecialReset(void)
{
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GUI, "Special reset");
	emit sig_vm_specialreset();
}

void Ui_MainWindowBase::do_emu_full_speed(bool flag)
{
	config.full_speed = flag;
}


void Ui_MainWindowBase::OnCpuPower(int mode)
{
	config.cpu_power = mode;
	emit sig_emu_update_config();
}

#include <QClipboard>
void Ui_MainWindowBase::OnStartAutoKey(void)
{
	QString ctext;
	QClipboard *clipBoard = QApplication::clipboard();
	ctext = clipBoard->text();
	emit sig_start_auto_key(ctext);
}

void Ui_MainWindowBase::OnStopAutoKey(void)
{
	emit sig_stop_auto_key();
}

// Note: Will move launching/exing debugger.
