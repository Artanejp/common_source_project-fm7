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

#include "../../fileio.h"

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
	CSP_Debugger_Tmpl(OSD_BASE* p_osd, QWidget *parent);
	~CSP_Debugger_Tmpl();
	//virtual void closeEvent(QCloseEvent *event);
	QStringList &get_complete_list();
	virtual void resizeEvent(QResizeEvent *event);
public slots:
	void stop_polling();
	void put_string(QString);
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
signals:
	void sig_put_string(QString);
	void sig_run_command(QString);
	void sig_finished();
	void sig_start_debugger();
	void sig_call_debugger(QString);
	void sig_close_debugger(void);
	void sig_apply_complete_list(QStringList);
};

QT_END_NAMESPACE	
#endif
