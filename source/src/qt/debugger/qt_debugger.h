/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.04.09 -
	History: 09 Apr, 2015 : Initial from Takeda.Toshiya's w32_debugger.cpp.
	[ debugger console ]
*/

#ifndef _CSP_QT_DEBUGGER_H
#define _CSP_QT_DEBUGGER_H

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <QMetaObject>
#include <QTimer>
#include <QCloseEvent>

#include "qt_debugger_tmpl.h"
#include "../../emu.h"
#include "../../vm/device.h"
#include "../../vm/debugger.h"
#include "../../vm/vm.h"
#include "../../fileio.h"

#if defined(USE_DEBUGGER)
#include "debugger_thread.h"
#endif

QT_BEGIN_NAMESPACE	
class CSP_Debugger : public CSP_Debugger_Tmpl
{
	Q_OBJECT
 protected:
	CSP_DebuggerThread *main_thread;
 public:
	CSP_Debugger(QWidget *parent);
	~CSP_Debugger();
#if defined(USE_DEBUGGER)
	debugger_thread_t debugger_thread_param;
#endif	
	void closeEvent(QCloseEvent *event);
	
public slots:
	void doExit(void);
	void doExit2(void);
	void call_debugger(void);
	void run(void);
	void do_destroy_thread(void);
signals:
	void sig_put_string(QString);
	void sig_run_command(QString);
	void sig_finished();
	void sig_start_debugger();
	void sig_call_debugger(QString);
	void sig_close_debugger(void);
	void sig_stop_debugger(void);
	void sig_run_debugger(void);
};

QT_END_NAMESPACE	
#endif
