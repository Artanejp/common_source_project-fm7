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
//#include <QThread>
//#include <QMainWindow>


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


void CSP_Debugger::doExit2(void)
{
	emit sig_close_debugger();
}

void CSP_Debugger::doExit(void)
{
	DEVICE *cpu = debugger_thread_param.vm->get_cpu(debugger_thread_param.cpu_index);
	DEBUGGER *debugger = (DEBUGGER *)cpu->get_debugger();
	debugger_thread_param.request_terminate = true;
	
	try {
		debugger->now_debugging = debugger->now_going = debugger->now_suspended = false;
	} catch(...) {
	}
	// release console
	debugger_thread_param.running = false;
	//QMetaObject::invokeMethod(main_thread, "quit", Qt::DirectConnection);
	//text_command->close();
	//main_thread->exit();
	//if(!main_thread->isFinished()) {
	//	main_thread->terminate();
	//}
	//delete main_thread;
	emit sig_finished();
}

void CSP_Debugger::stop_polling()
{
	//poll_stop = true;
}

void CSP_Debugger::call_debugger(void)
{
	//emit sig_call_debugger(text_command->text());
	main_thread->call_debugger(text_command->text());
}

void CSP_Debugger::run(void)
{
	main_thread = new CSP_DebuggerThread(NULL, &debugger_thread_param);
	main_thread->setObjectName(QString::fromUtf8("Debugger"));
	main_thread->moveToThread(main_thread);
	//main_thread = new CSP_DebuggerThread(this, &debugger_thread_param);
	
	connect(text_command, SIGNAL(editingFinished()), this, SLOT(call_debugger()));
	connect(this, SIGNAL(sig_call_debugger(QString)), main_thread, SLOT(call_debugger(QString)));
	
	connect(main_thread, SIGNAL(sig_text_clear()), this, SLOT(cmd_clear()));
	connect(main_thread, SIGNAL(sig_put_string(QString)), this, SLOT(put_string(QString)));
	
	connect(main_thread, SIGNAL(finished()), this, SLOT(doExit()));
	connect(main_thread, SIGNAL(quit_debugger_thread()), this, SLOT(doExit()));
	
	connect(this, SIGNAL(sig_finished()), this, SLOT(close()));
	connect(this, SIGNAL(destroyed()), this, SLOT(doExit()));
	connect(this, SIGNAL(sig_close_debugger()), main_thread, SLOT(quit_debugger()));
	
	//connect(parent_object, SIGNAL(quit_debugger_thread()), this, SLOT(doExit2()));
	connect(parent_object, SIGNAL(quit_debugger_thread()), this, SLOT(close()));
								  
	connect(this, SIGNAL(sig_start_debugger()), main_thread, SLOT(start()));
	main_thread->start();
	//emit sig_start_debugger();
}

void CSP_Debugger::closeEvent(QCloseEvent *event)
{
	main_thread->terminate();
	doExit();
}

CSP_Debugger::CSP_Debugger(QWidget *parent) : QWidget(parent, Qt::Window)
{
	widget = this;
	
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
	//connect(text_command, SIGNAL(editingFinished()), this, SLOT(call_debugger()));
	
	VBoxWindow = new QVBoxLayout;

	VBoxWindow->addWidget(text);
	VBoxWindow->addWidget(text_command);
	this->setLayout(VBoxWindow);
	this->resize(640, 500);
}


CSP_Debugger::~CSP_Debugger()
{
	
}
   

void EMU::initialize_debugger()
{
	now_debugging = false;
	hDebugger = NULL;
}

void EMU::release_debugger()
{
	close_debugger();
}

void EMU::open_debugger(int cpu_index)
{
}

void EMU::close_debugger()
{
}

bool EMU::debugger_enabled(int cpu_index)
{
	return (vm->get_cpu(cpu_index) != NULL && vm->get_cpu(cpu_index)->get_debugger() != NULL);
}

#endif

