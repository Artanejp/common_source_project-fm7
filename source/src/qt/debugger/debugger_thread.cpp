
#include <QThread>
#include <QMetaObject>
#include <math.h>
#include "./qt_debugger.h"


void CSP_DebuggerThread::quit_debugger()
{
	debugger->now_going = false;
	d_params->running = false;
	d_params->request_terminate = true;
	emit quit_debugger_thread();
}


extern int debugger_thread(void *p);

void CSP_DebuggerThread::run()
{
	QString str = QString::fromUtf8(_T("Debugger CPU #")) + QString::number(cpu_index);
	emit sig_set_title(str);

	pausing = false;
	d_params->running = true;
	d_params->request_terminate = false;
	dasm_addr = cpu->get_next_pc();
	debugger->now_going = false;
	debugger->now_debugging = true;
   
        debugger_thread((void *)d_params);
	this->quit();
}

CSP_DebuggerThread::CSP_DebuggerThread(QObject *parent, debugger_thread_t *th) : QThread(parent)
{
	d_params = th;
	cpu = d_params->vm->get_cpu(d_params->cpu_index);
	cpu_index = d_params->cpu_index;
	debugger = (DEBUGGER *)cpu->get_debugger();
	
	dasm_addr = 0;
	connect(this, SIGNAL(started()), this, SLOT(run()));
}

CSP_DebuggerThread::~CSP_DebuggerThread()
{
}
