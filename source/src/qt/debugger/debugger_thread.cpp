/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.04.09 -
	History: 09 Apr, 2015 : Initial from Takeda.Toshiya's w32_debugger.cpp.
	[ debugger thread]
*/

#include <QThread>
#include <QMetaObject>
#include <math.h>
#include "./debugger_thread.h"

void CSP_DebuggerThread::quit_debugger()
{
#if defined(USE_DEBUGGER)
	emu->close_debugger();
	// release console
#endif	
	emit quit_debugger_thread();
}

#if defined(USE_DEBUGGER)	
extern void *debugger_thread(void *p);
#endif

void CSP_DebuggerThread::run_debugger(void)
{
//	emu->open_debugger(cpu_index);
}

CSP_DebuggerThread::CSP_DebuggerThread(QObject *parent, void *th) : QThread(parent)
{
	dasm_addr = 0;
	cpu_index = 0;
	cpu = NULL;
	pausing = false;
#if defined(USE_DEBUGGER)	
	d_params = (debugger_thread_t *)th;
	cpu = d_params->vm->get_cpu(d_params->cpu_index);
	cpu_index = d_params->cpu_index;
	debugger = (DEBUGGER *)cpu->get_debugger();
#endif	
	//connect(this, SIGNAL(started()), this, SLOT(run()));
	QString str = QString::fromUtf8(_T("Debugger CPU #")) + QString::number(cpu_index);
	emit sig_set_title(str);

	pausing = false;
#if defined(USE_DEBUGGER)	
	d_params->running = true;
	d_params->request_terminate = false;
	dasm_addr = cpu->get_next_pc();
	//debugger->now_going = false;
	//debugger->now_debugging = true;
	emu->open_debugger(cpu_index);
	//debugger_thread((void *)d_params);
#endif
	//this->exec();
	//this->quit();
}

CSP_DebuggerThread::~CSP_DebuggerThread()
{
	emu->close_debugger();
}
