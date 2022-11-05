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
#include <QObject>
#include <QMetaObject>
#include <QApplication>
#include "../../emu_template.h"
#include "qt_debugger.h"
#include "../gui/qt_lineeditplus.h"


void CSP_Debugger::doExit(void)
{
	emit sig_finished();
}

void CSP_Debugger::doExit2(void)
{
	emit sig_finished();
}


CSP_Debugger::CSP_Debugger(EMU_TEMPLATE* p_emu, QWidget *parent) : CSP_Debugger_Tmpl(p_emu, parent)
{
	
}

CSP_Debugger::~CSP_Debugger()
{
}


