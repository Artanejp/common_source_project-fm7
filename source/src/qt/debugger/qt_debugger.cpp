/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.04.09 -
	History: 09 Apr, 2015 : Initial from Takeda.Toshiya's w32_debugger.cpp.
	[ debugger console ]
*/

#include <stdio.h>
#include <string.h>

//#include <unistd.h>
#include <fcntl.h>
#include <QObject>
#include <QMetaObject>
#include <QApplication>

#include "qt_debugger.h"

void CSP_Debugger::doExit(void)
{
	emit sig_finished();
}

void CSP_Debugger::doExit2(void)
{
	emit sig_finished();
}

void CSP_Debugger::call_debugger(void)
{
#if defined(USE_DEBUGGER)
	OSD *osd = debugger_thread_param.osd;
	osd->do_set_input_string(text_command->text());
#endif	
	cmd_clear();
}

void CSP_Debugger::run(void)
{
	connect(text_command, SIGNAL(editingFinished()), this, SLOT(call_debugger()));
	connect(parent_object, SIGNAL(quit_debugger_thread()), this, SLOT(close()));
	
#if defined(USE_DEBUGGER)
	OSD *osd = debugger_thread_param.osd;
	connect(osd, SIGNAL(sig_close_console()), this, SLOT(do_destroy_thread()));
	connect(this, SIGNAL(sig_call_debugger(QString)), osd, SLOT(do_set_input_string(QString)));
	connect(osd, SIGNAL(sig_put_string_debugger(QString)), this, SLOT(put_string(QString)));
	if(emu != NULL) {
		emu->open_debugger(debugger_thread_param.cpu_index);
	} else {
		QString mes = QApplication::translate("Debugger", "Emulator still not start\nPlease wait.", 0);
		put_string(mes);
	}
#endif
}

void CSP_Debugger::closeEvent(QCloseEvent *event)
{
	//emit sig_close_debugger();
	//event->ignore();
#if defined(USE_DEBUGGER)
	debugger_thread_param.request_terminate = true;
	if(emu != NULL) {
		debugger_thread_t *d_params = &debugger_thread_param;
		DEVICE *cpu = d_params->vm->get_cpu(d_params->cpu_index);
		uint32_t cpu_index = d_params->cpu_index;
		DEBUGGER *debugger = (DEBUGGER *)cpu->get_debugger();
		if(emu->is_debugger_enabled(cpu_index)) {
			emu->close_debugger();
			//	debugger->now_debugging = false;
		}
	}
#endif
	event->accept();
	emit sig_finished();
}

void CSP_Debugger::do_destroy_thread(void)
{
	this->close();
}

CSP_Debugger::CSP_Debugger(QWidget *parent) : CSP_Debugger_Tmpl(parent)
{
}

CSP_Debugger::~CSP_Debugger()
{
#if defined(USE_DEBUGGER)
	if(emu != NULL) emu->close_debugger();
#endif
}


