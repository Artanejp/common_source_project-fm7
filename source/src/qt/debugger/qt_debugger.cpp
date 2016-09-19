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
	connect(this, SIGNAL(sig_finished()), this, SLOT(close()));
	connect(this, SIGNAL(destroyed()), this, SLOT(doExit()));
	connect(parent_object, SIGNAL(quit_debugger_thread()), this, SLOT(close()));
	
#if defined(USE_DEBUGGER)
	main_thread = new CSP_DebuggerThread(NULL, &debugger_thread_param);
	OSD *osd = debugger_thread_param.osd;
	main_thread->setObjectName(QString::fromUtf8("Debugger"));
	main_thread->moveToThread(main_thread);
	connect(this, SIGNAL(sig_call_debugger(QString)), osd, SLOT(do_set_input_string(QString)));
	connect(osd, SIGNAL(sig_put_string_debugger(QString)), this, SLOT(put_string(QString)));
	
	connect(main_thread, SIGNAL(finished()), this, SLOT(doExit()));
	connect(main_thread, SIGNAL(quit_debugger_thread()), this, SLOT(doExit()));
	connect(this, SIGNAL(sig_close_debugger()), main_thread, SLOT(quit_debugger()));
	main_thread->start();
#endif
}

void CSP_Debugger::closeEvent(QCloseEvent *event)
{
	//emit sig_close_debugger();
	event->ignore();
}

void CSP_Debugger::do_destroy_thread(void)
{
#if defined(USE_DEBUGGER)
	if(main_thread != NULL) {
		if(main_thread->isRunning()) {
			main_thread->quit_debugger();
			main_thread->terminate();
		}
		delete main_thread;
	}
	main_thread = NULL;
#endif
	this->close();
}

CSP_Debugger::CSP_Debugger(QWidget *parent) : CSP_Debugger_Tmpl(parent)
{
#if defined(USE_DEBUGGER)	
	main_thread = NULL;
#endif
}

CSP_Debugger::~CSP_Debugger()
{
#if defined(USE_DEBUGGER)
	if(main_thread != NULL) {
		if(main_thread->isRunning()) {
			main_thread->quit_debugger();
			main_thread->terminate();
		}
		delete main_thread;
	}
#endif
}


