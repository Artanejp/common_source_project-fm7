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

#include "../../emu.h"
#include "../../vm/device.h"
#include "../../vm/debugger.h"
#include "../../vm/vm.h"
#include "../../fileio.h"

#include "../../qt/3rdparty/qtermwidget/lib/qtermwidget.h"

class CSP_Debugger : public QObject 
{
 protected:
	void my_printf(FILE *hStdOut, const _TCHAR *format, ...);
	void my_putch(FILE *hStdOut, _TCHAR c);
	uint32 my_hexatoi(_TCHAR *str);
	break_point_t *get_break_point(DEBUGGER *debugger, _TCHAR *command);

 public:
	debugger_thread_t debugger_thread_param;
	void start(void) {
		doWork(this);
	}
public slots:
	void doWork(QObject *parent);
	void doExit(void);
};
