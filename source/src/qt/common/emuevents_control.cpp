

#include "qt_emuevents.h"
#include "qt_main.h"
#include "qt_dialogs.h"
#include "agar_logger.h"

extern EMU *emu;

void Ui_MainWindow::OnReset(void)
{
	AGAR_DebugLog(AGAR_LOG_INFO, "Reset");
	emit sig_vm_reset();
}

void Ui_MainWindow::OnSpecialReset(void)
{
#ifdef USE_SPECIAL_RESET
	AGAR_DebugLog(AGAR_LOG_INFO, "Special Reset");
	emit sig_vm_specialreset();
#endif
}

#ifdef USE_STATE
void Ui_MainWindow::OnLoadState(void) // Final entry of load state.
{
	emit sig_vm_loadstate();
}

void Ui_MainWindow::OnSaveState(void)
{
	emit sig_vm_savestate();
}
#endif
#ifdef USE_BOOT_MODE
#endif

#ifdef USE_CPU_TYPE
#endif

void Ui_MainWindow::OnCpuPower(int mode)
{
	config.cpu_power = mode;
	emit sig_emu_update_config();
}

#ifdef USE_AUTO_KEY
#include <QClipboard>
void Ui_MainWindow::OnStartAutoKey(void)
{
	QString ctext;
	QClipboard *clipBoard = QApplication::clipboard();
	ctext = clipBoard->text();
	emit sig_start_auto_key(ctext);
}

void Ui_MainWindow::OnStopAutoKey(void)
{
	emit sig_stop_auto_key();
}
#endif
#ifdef USE_DEBUGGER
#include <../debugger/qt_debugger.h>

void Ui_MainWindow::OnOpenDebugger(int no)
{
	if((no < 0) || (no > 3)) return;

	if(emu->now_debugging) 	this->OnCloseDebugger();
	emu->open_debugger(no);
	OSD *p_osd = emu->debugger_thread_param.osd;
	emu->hDebugger = new CSP_Debugger(this, p_osd);
	connect(p_osd, SIGNAL(sig_put_string_debugger(QString)), emu->hDebugger, SLOT(put_string(QString)), Qt::DirectConnection);
	emu->hDebugger->show();
	emu->hDebugger->run();
}

void Ui_MainWindow::OnCloseDebugger(void )
{
	if(emu->now_debugging) {
		emu->close_debugger();
		if(emu->hDebugger != NULL) delete emu->hDebugger;
		emu->hDebugger = NULL;
		emu->now_debugging = false;
	}
}
#endif

// Implement LASER-DISC, BINARY
//

void OnStartRecordScreen(int num)
{
	const int fps[3] = {60, 30, 15};
	if((num < 0) || (num > 2)) return;
	if(emu) {
		//emit sig_emu_start_rec_screen();
		emu->start_rec_sound();
		if(!emu->start_rec_video(fps[num])) {
			emu->stop_rec_sound();
		}
	}
}
void OnStopRecordScreen(void)
{
	if(emu) {
		//emit sig_emu_stop_rec_screen();
		emu->stop_rec_video();
		emu->stop_rec_sound();
	}
}

void OnScreenCapture(QWidget *parent)
{
	if(emu) emu->capture_screen();
}

void OnFullScreen(QMainWindow *MainWindow, QWidget *drawspace, int mode)
{
}

