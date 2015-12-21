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
#include <QCloseEvent>

#include "../../emu.h"
#include "../../vm/device.h"
#include "../../vm/debugger.h"
#include "../../vm/vm.h"
#include "../../fileio.h"

#define MAX_COMMAND_LEN	64
	
class CSP_DebuggerThread : public QThread
{
	Q_OBJECT

protected:
	debugger_thread_t *d_params;
	DEBUGGER *debugger;
	DEVICE *cpu;
	uint32 cpu_index;
	bool pausing;
	
	FILEIO *logfile;
	QTimer *trap_timer;
	QString prev_command;
	uint32 dump_addr;
	uint32 dasm_addr;
	int trace_steps;
	bool request_terminate;
	
	break_point_t *get_break_point(DEBUGGER *debugger, _TCHAR *command);
	uint32 my_hexatoi(_TCHAR *str);
	void my_putch(_TCHAR c);
	void my_printf(const _TCHAR *format, ...);
	void getRegisterInfo();
	void display_break_status(void);
	void display_pc(void);
	
public:
	CSP_DebuggerThread(QObject *parent, debugger_thread_t *th);
	~CSP_DebuggerThread();
public slots:
	void run();
	virtual int debugger_main(QString command);
	void call_debugger(QString);
	void check_trap();
	void display_break_point();
	void quit_debugger();
	void do_string_input(QString s);
signals:
	int sig_start_trap();
	int sig_end_trap();
	int sig_text_clear();
	int sig_put_string(QString);
	int quit_debugger_thread();
	void sig_set_title(QString);
};

class CSP_Debugger : public QWidget
{
	Q_OBJECT
 private:
	QObject *parent_object;
	QWidget *widget;
	QTextEdit *text;
	QLineEdit *text_command;
	QVBoxLayout *VBoxWindow;
	CSP_DebuggerThread *main_thread;
	
 protected:
	//QFont font;// = QApplication::font();
	//QMainWindow  *debug_window;
 public:
	CSP_Debugger(QWidget *parent);
	~CSP_Debugger();
	debugger_thread_t debugger_thread_param;
	void closeEvent(QCloseEvent *event);
	
public slots:
	void doExit(void);
	void doExit2(void);
	void stop_polling();
	void put_string(QString);
	void cmd_clear();
	void call_debugger(void);
	void run(void);
signals:
	void sig_put_string(QString);
	void sig_run_command(QString);
	void sig_finished();
	void sig_start_debugger();
	void sig_call_debugger(QString);
	void sig_close_debugger(void);
};

