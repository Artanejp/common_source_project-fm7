/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.04.09 -
	History: 09 Apr, 2015 : Initial from Takeda.Toshiya's w32_debugger.cpp.
	[ debugger console ]
*/
#ifndef _CSP_DEBUGGER_THREAD_H
#define _CSP_DEBUGGER_THREAD_H

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <QMetaObject>
#include <QThread>
#include <QCloseEvent>

#include "../../vm/vm.h"
#if defined(USE_DEBUGGER)	
#include "../../emu.h"
#endif
#include "../../vm/device.h"
#if defined(USE_DEBUGGER)	
#include "../../vm/debugger.h"
#include "../../vm/vm.h"
#endif
#include "../../fileio.h"

#define MAX_COMMAND_LEN	64

QT_BEGIN_NAMESPACE	
class CSP_DebuggerThread : public QThread
{
	Q_OBJECT
protected:
#if defined(USE_DEBUGGER)	
	debugger_thread_t *d_params;
	DEBUGGER *debugger;
#endif	
	DEVICE *cpu;
	uint32_t cpu_index;
	bool pausing;
	
	uint32_t dasm_addr;
public:
	CSP_DebuggerThread(QObject *parent, void *th);
	~CSP_DebuggerThread();
public slots:
	void run_debugger();
	void quit_debugger();
signals:
	int quit_debugger_thread();
	void sig_set_title(QString);
};
QT_END_NAMESPACE	

#endif
