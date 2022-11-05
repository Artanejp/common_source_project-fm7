/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.04.09 -
	History: 09 Apr, 2015 : Initial from Takeda.Toshiya's w32_debugger.cpp.
	[ debugger console ]
*/

#ifndef _CSP_QT_DEBUGGER_H
#define _CSP_QT_DEBUGGER_H

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <QMetaObject>
#include <QTimer>
#include <QCloseEvent>

#include "qt_debugger_tmpl.h"
#include "../../emu_template.h"

#include "../../vm/vm_template.h"

#include "../../vm/device.h"
#include "../../vm/debugger.h"

#include "../../fileio.h"

QT_BEGIN_NAMESPACE
class CSP_Debugger : public CSP_Debugger_Tmpl
{
	Q_OBJECT
 protected:

 public:
	CSP_Debugger(EMU_TEMPLATE* p_emu, QWidget *parent = nullptr);
	~CSP_Debugger();

public slots:
	void doExit(void);
	void doExit2(void);
};

QT_END_NAMESPACE	
#endif
