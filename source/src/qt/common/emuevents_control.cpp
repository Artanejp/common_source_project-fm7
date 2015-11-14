

#include "qt_emuevents.h"
#include "qt_main.h"
#include "qt_dialogs.h"
#include "emu_utils.h"
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
	if(emu) {
		emu->update_config();
	}
}

#ifdef USE_AUTO_KEY
void Ui_MainWindow::OnStartAutoKey(void)
{
	QString ctext;
	ctext = ClipBoard->text();
	emit sig_start_auto_key(ctext);
	ClipBoard->clear();
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
	//emu->open_debugger(no);
	VM *vm = emu->getVM();
	if(emu->now_debugging) 	this->OnCloseDebugger();
	if(!(emu->now_debugging && emu->debugger_thread_param.cpu_index == no)) {
		//emu->close_debugger();
		if(vm->get_cpu(no) != NULL && vm->get_cpu(no)->get_debugger() != NULL) {
			
			emu->hDebugger = new CSP_Debugger(this);
			QString objNameStr = QString("EmuDebugThread");
			emu->hDebugger->setObjectName(objNameStr);
			emu->hDebugger->debugger_thread_param.emu = emu;
			emu->hDebugger->debugger_thread_param.vm = vm;
			emu->hDebugger->debugger_thread_param.cpu_index = no;
			emu->stop_rec_sound();
			emu->stop_rec_video();
			emu->now_debugging = true;
			connect(this, SIGNAL(quit_debugger_thread()), emu->hDebugger, SLOT(doExit()));
			//connect(this, SIGNAL(quit_debugger_thread()), emu->hDebugger, SLOT(close()));
			connect(emu->hDebugger, SIGNAL(sig_finished()), this, SLOT(OnCloseDebugger()));
			connect(emu->hDebugger, SIGNAL(sig_put_string(QString)), emu->hDebugger, SLOT(put_string(QString)));
			emu->hDebugger->show();
			emu->hDebugger->run();
		}
	}
}

void Ui_MainWindow::OnCloseDebugger(void )
{
//	emu->close_debugger();
	if(emu->now_debugging) {
		if(emu->hDebugger->debugger_thread_param.running) {
			emit quit_debugger_thread();
			//emu->hDebugger->wait();
		}
		delete emu->hDebugger;
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
		emu->start_rec_sound();
		if(!emu->start_rec_video(fps[num])) {
			emu->stop_rec_sound();
		}
	}
}
void OnStopRecordScreen(void)
{
	if(emu) {
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

