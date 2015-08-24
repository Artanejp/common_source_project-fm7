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
//#include <QMainWindow>
#include <QThread>
#include <QWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QTimer>

#include "../../emu.h"
#include "../../vm/device.h"
#include "../../vm/debugger.h"
#include "../../vm/vm.h"
#include "../../fileio.h"

#define MAX_COMMAND_LEN	64
	
class CSP_Debugger : public QWidget
{
	Q_OBJECT
 private:
	QObject *parent_object;
	QWidget *widget;
	QTextEdit *text;
	QLineEdit *text_command;
	QTimer *trap_timer;
	QVBoxLayout *VBoxWindow;
	
	void Sleep(uint32_t tick);
	break_point_t *get_break_point(DEBUGGER *debugger, _TCHAR *command);
	uint32 my_hexatoi(_TCHAR *str);
	void my_putch(_TCHAR c);
	void my_printf(const _TCHAR *format, ...);
	
	QString prev_command;
	uint32 dump_addr;
	uint32 dasm_addr;
	int trace_steps;
	
	bool polling;
	
	bool running;
 protected:
	//QFont font;// = QApplication::font();
	//QMainWindow  *debug_window;
 public:
	CSP_Debugger(QWidget *parent);
	~CSP_Debugger();
	debugger_thread_t debugger_thread_param;
	void run() { doWork("");}
public slots:
	void doWork(const QString &param);
	void doExit(void);
	virtual int debugger_main(QString command);
	void stop_polling();
	void put_string(QString);
	void call_debugger(void);
	void check_trap(void);
signals:
	void quit_debugger_thread();
	void sig_put_string(QString);
	void sig_run_command(QString);
	void sig_stop_polling();
	void sig_finished();
	void sig_start_trap();
	void sig_end_trap();
};
