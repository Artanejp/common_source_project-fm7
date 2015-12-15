/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.04.09 -
	History: 09 Apr, 2015 : Initial from Takeda.Toshiya's w32_debugger.cpp.
	[ debugger console ]
*/

#include <stdio.h>
#include <string.h>

//#include <unistd.h>
#include <fcntl.h>
#include <QObject>
#include <QMetaObject>

//#include "res/resource.h"
#include "../../emu.h"
#include "../../vm/device.h"
#include "../../vm/debugger.h"
#include "../../vm/vm.h"
#include "../../fileio.h"
#include "qt_debugger.h"
#include "osd.h"


#ifdef USE_DEBUGGER

void CSP_Debugger::put_string(QString s)
{
	text->insertPlainText(s);
	text->moveCursor(QTextCursor::End);
}

void CSP_Debugger::cmd_clear()
{
	text_command->clear();
	text->moveCursor(QTextCursor::End);
}

void CSP_Debugger::doExit(void)
{
	//p_emu->close_debugger();
	emit sig_finished();
}

void CSP_Debugger::stop_polling()
{
	//poll_stop = true;
}

void CSP_Debugger::call_debugger(void)
{
	emit sig_call_debugger(text_command->text());
}

void CSP_Debugger::run(void)
{
	connect(text_command, SIGNAL(editingFinished()), this, SLOT(call_debugger()));
	connect(this, SIGNAL(sig_call_debugger(QString)), p_osd, SLOT(do_write_inputdata(QString)), Qt::DirectConnection);
	
	connect(p_osd, SIGNAL(sig_debugger_finished()), this, SLOT(doExit()));
	connect(this, SIGNAL(sig_finished()), p_osd, SLOT(do_close_debugger_thread()));
	connect(this, SIGNAL(destroyed()), this, SLOT(doExit()));
	connect(parent_object, SIGNAL(quit_debugger_thread()), this, SLOT(close()));
	connect(this, SIGNAL(sig_finished()), this, SLOT(close()));
	
	//emit sig_start_debugger();
}

void CSP_Debugger::closeEvent(QCloseEvent *event)
{
	doExit();
}

CSP_Debugger::CSP_Debugger(QWidget *parent, OSD *osd) : QWidget(parent, Qt::Window)
{
	widget = this;
	p_osd = osd;
	
	parent_object = parent;
	text = new QTextEdit(this);
	text->setReadOnly(true);
	text->setLineWrapMode(QTextEdit::WidgetWidth);
	//text->setCenterOnScroll(true);

	text_command = new QLineEdit(this);
	text_command->setEchoMode(QLineEdit::Normal);
	text_command->setMaxLength(1024);
	text_command->setReadOnly(false);
	text_command->setEnabled(true);
	text_command->clear();
	
	VBoxWindow = new QVBoxLayout;

	VBoxWindow->addWidget(text);
	VBoxWindow->addWidget(text_command);
	this->setLayout(VBoxWindow);
	this->resize(640, 500);
}


CSP_Debugger::~CSP_Debugger()
{
	
}
   


#endif

