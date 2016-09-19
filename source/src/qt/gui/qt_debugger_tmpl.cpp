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
#include "qt_debugger_tmpl.h"
//#include <QThread>
//#include <QMainWindow>


void CSP_Debugger_Tmpl::put_string(QString s)
{
	text->insertPlainText(s);
	text->moveCursor(QTextCursor::End);
}

void CSP_Debugger_Tmpl::cmd_clear()
{
	text_command->clear();
	text->moveCursor(QTextCursor::End);
}

void CSP_Debugger_Tmpl::stop_polling()
{
	//poll_stop = true;
}

void CSP_Debugger_Tmpl::run(void)
{
	connect(this, SIGNAL(sig_finished()), this, SLOT(close()));
	connect(parent_object, SIGNAL(quit_debugger_thread()), this, SLOT(close()));
}


CSP_Debugger_Tmpl::CSP_Debugger_Tmpl(QWidget *parent) : QWidget(parent, Qt::Window)
{
	widget = this;
	
	parent_object = parent;
	text = new QTextEdit(this);
	text->setReadOnly(true);
	text->setLineWrapMode(QTextEdit::WidgetWidth);

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


CSP_Debugger_Tmpl::~CSP_Debugger_Tmpl()
{
	
}
