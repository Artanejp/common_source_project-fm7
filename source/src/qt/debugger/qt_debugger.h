/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.04.09 -
	History: 09 Apr, 2015 : Initial from Takeda.Toshiya's w32_debugger.cpp.
	[ debugger console ]
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <QMetaObject>
#include <QMainWindow>
#include <QThread>

#include "../../emu.h"
#include "../../vm/device.h"
#include "../../vm/debugger.h"
#include "../../vm/vm.h"
#include "../../fileio.h"

class QTermWidget;

class CSP_Debugger : public QThread 
{
	Q_OBJECT
 private:
	void Sleep(uint32_t tick);
	break_point_t *get_break_point(DEBUGGER *debugger, _TCHAR *command);
	uint32 my_hexatoi(_TCHAR *str);
	void my_putch(FILE *hStdOut, _TCHAR c);
	void my_printf(FILE *hStdOut, const _TCHAR *format, ...);
 protected:
	QFont font;// = QApplication::font();
	QMainWindow  *debug_window;
 public:
	CSP_Debugger(QObject *parent);
	~CSP_Debugger();
	debugger_thread_t debugger_thread_param;
	virtual int debugger_main();
	void run() { doWork("");}
public slots:
	void doWork(const QString &param);
	void doExit(void);
signals:
	void quit_debugger_thread();

};
