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
#include <SDL2/SDL.h>

#include "../../emu.h"
#include "../../vm/device.h"
#include "../../vm/debugger.h"
#include "../../vm/vm.h"
#include "../../fileio.h"

class QTermWidget;

class CSP_Debugger : public QObject 
{
	Q_OBJECT
 protected:
	QFont font;// = QApplication::font();
	QMainWindow  *debug_window;
	QTermWidget  *hConsole;
 public:
	CSP_Debugger(QObject *parent);
	~CSP_Debugger();
	debugger_thread_t debugger_thread_param;
        SDL_Thread *thread;
public slots:
        void start(void) {
		doWork(this);
	}
	void doWork(QObject *parent);
	void doExit(void);
};
