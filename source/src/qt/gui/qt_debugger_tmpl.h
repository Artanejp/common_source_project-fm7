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

#include <QString>
#include <QStringList>

#include <list>
#include <string>
#include <memory>

#include "../../fileio.h"
#include "../../config.h"
#include "../../emu_template.h"

QT_BEGIN_NAMESPACE	

class QTextEdit;
class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QFontDialog;
class QResizeEvent;

class CSP_DebuggerThread;
class QLineEditPlus;
class OSD_BASE;
class DLL_PREFIX CSP_Debugger_Tmpl : public QWidget
{
	Q_OBJECT
 protected:
	config_t *p_cfg;
	EMU_TEMPLATE* emu;
	
	QObject *parent_object;
	QWidget *widget;
	QTextEdit *text;
	QLineEditPlus *text_command;
	QPushButton *call_font_dialog;
	QFontDialog *font_dialog;
	QVBoxLayout *VBoxWindow;
	QHBoxLayout *TailButtons;

	bool string_is_strong;
	QString text_color;
	QStringList complete_list;

public:
	CSP_Debugger_Tmpl(EMU_TEMPLATE* p_emu, QWidget *parent = nullptr);
	~CSP_Debugger_Tmpl();
	debugger_thread_t debugger_thread_param;

	QStringList &get_complete_list();
	void resizeEvent(QResizeEvent *event);
	void closeEvent(QCloseEvent *event);										 
public slots:
	void stop_polling();
	void cmd_clear();
	virtual void run(void);
	void set_string_attr(QString color, bool is_strong);
	void add_complete_list(_TCHAR *index);
	void add_complete_list(QString index);
	void set_complete_list(std::list<std::string> *list);
	void set_complete_list(QStringList list);
	
	void clear_complete_list();
	void apply_complete_list();
	void set_font(const QFont &font);
	void rise_font_dialog();

	void do_destroy_thread();
	void put_string(QString str);
	void call_debugger();

signals:
	void sig_put_string(QString);
	void sig_run_command(QString);
	void sig_finished();
	void sig_start_debugger();
	void sig_call_debugger(QString);
	void sig_close_debugger(void);
	void sig_apply_complete_list(QStringList);

	void sig_set_input_string(QString);
	
	void sig_stop_debugger(void);
	void sig_run_debugger(void);
};

QT_END_NAMESPACE	
#endif
