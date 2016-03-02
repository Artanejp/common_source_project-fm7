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
	uint32_t cpu_index;
	bool pausing;
	
	uint32_t dasm_addr;
public:
	CSP_DebuggerThread(QObject *parent, debugger_thread_t *th);
	~CSP_DebuggerThread();
public slots:
	void run();
	void quit_debugger();
signals:
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

