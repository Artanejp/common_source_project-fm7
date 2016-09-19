/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.04.09 -
	History: 09 Apr, 2015 : Initial from Takeda.Toshiya's w32_debugger.cpp.
	[ debugger console ]
*/

#ifndef _CSP_QT_DEBUGGER_TMPL_H
#define _CSP_QT_DEBUGGER_TMPL_H

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <QMetaObject>
#include <QThread>
#include <QWidget>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QString>
#include <QStringList>
#include <QVBoxLayout>
#include <QTimer>
#include <QCloseEvent>

#include "../../fileio.h"

QT_BEGIN_NAMESPACE	

class CSP_DebuggerThread;
class CSP_Debugger_Tmpl : public QWidget
{
	Q_OBJECT
 protected:
	QObject *parent_object;
	QWidget *widget;
	QTextEdit *text;
	QLineEdit *text_command;
	QVBoxLayout *VBoxWindow;
 public:
	CSP_Debugger_Tmpl(QWidget *parent);
	~CSP_Debugger_Tmpl();
	//virtual void closeEvent(QCloseEvent *event);
public slots:
	void stop_polling();
	void put_string(QString);
	void cmd_clear();
	virtual void run(void);
signals:
	void sig_put_string(QString);
	void sig_run_command(QString);
	void sig_finished();
	void sig_start_debugger();
	void sig_call_debugger(QString);
	void sig_close_debugger(void);
};

QT_END_NAMESPACE	
#endif
