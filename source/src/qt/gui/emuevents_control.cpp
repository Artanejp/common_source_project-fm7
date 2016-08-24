

#include "qt_emuevents.h"
#include "qt_main.h"
#include "qt_dialogs.h"
#include "agar_logger.h"

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

void Ui_MainWindowBase::OnLoadState(void) // Final entry of load state.
{
	emit sig_vm_loadstate();
}

void Ui_MainWindowBase::OnSaveState(void)
{
	emit sig_vm_savestate();
}

void Ui_MainWindowBase::OnCpuPower(int mode)
{
	using_flags->get_config_ptr()->cpu_power = mode;
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
