
/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -

	[ win32 main ] -> [ agar main ]
*/

#include <stdio.h>
#include <string>
#include <vector>
#include "common.h"
#include "fileio.h"

#include "emu.h"
#include "csp_logger.h"

#include "menuclasses.h"
#include "mainwidget.h"
#include "commonclasses.h"
#include "qt_main.h"
#include "emu_thread.h"
#include "joy_thread.h"
#include "draw_thread.h"

// emulation core

extern EMU* emu;
extern QApplication *GuiMain;
extern CSP_Logger *csp_logger;

// Start to define MainWindow.
extern class META_MainWindow *rMainWindow;
extern config_t config;
extern int MainLoop(int argc, char *argv[]);

#include <QApplication>
#include <qapplication.h>
#if defined(Q_OS_WIN)
//DLL_PREFIX_I void CSP_DebugHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
DLL_PREFIX_I void _resource_init(void);
DLL_PREFIX_I void _resource_free(void);
#else
extern void CSP_DebugHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
extern void _resource_init(void);
extern void _resource_free(void);
#endif
void CSP_DebugHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QString msg_type;
    switch (type) {
    case QtDebugMsg:
		msg_type = QString::fromUtf8("[Qt:DEBUG]");
        break;
    case QtInfoMsg:
		msg_type = QString::fromUtf8("[Qt:INFO]");
        break;
    case QtWarningMsg:
		msg_type = QString::fromUtf8("[Qt:WARN]");
        break;
    case QtCriticalMsg:
		msg_type = QString::fromUtf8("[Qt:CRITICAL]");
        break;
    case QtFatalMsg:
		msg_type = QString::fromUtf8("[Qt:FATAL]");
		break;
    }
	QString msgString = qFormatLogMessage(type, context, msg);
	QString nmsg_l1 = msg_type;
	QString nmsg_l2 = msg_type;
	nmsg_l2.append(" ");
	nmsg_l2.append(msgString);
	nmsg_l1.append(" In line ");
	nmsg_l1.append(context.line);
	nmsg_l1.append(" of ");
	nmsg_l1.append(context.line);
	nmsg_l1.append(" (Function: ");
	nmsg_l1.append(context.function);
	nmsg_l1.append(" )");

	if(csp_logger != NULL) {
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GUI, nmsg_l1.toLocal8Bit().constData());
		csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GUI, nmsg_l2.toLocal8Bit().constData());
	} else {
		fprintf(stderr,"%s\n", nmsg_l1.toLocal8Bit().constData());
		fprintf(stderr, "%s\n", nmsg_l2.toLocal8Bit().constData());
	}		
}

int main(int argc, char *argv[])
{
	int nErrorCode;
	/*
	 * Get current DIR
	 */
	csp_logger = NULL;
/*
 * アプリケーション初期化
 */
	_resource_init();
	qSetMessagePattern(QString::fromUtf8("[%{type}] %{message} \n   at line %{line} of %{file} : function %{function}\nBacktrace:\n %{backtrace separator=\"\n \" }"));
	qInstallMessageHandler(CSP_DebugHandler);
	printf("argc: %d\n", argc);
	for(int i = 0; i < argc; i++) {
		printf("argv[%d]=%s\n", i, argv[i]);
	}
	nErrorCode = MainLoop(argc, argv);
	_resource_free();
	if(csp_logger != NULL) delete csp_logger;
	
	return nErrorCode;
}

#if defined(Q_OS_WIN) 
/*
 * This is from WinMain at Qt5Core, v5.10.
 * see, qtbase/src/qtmain_win.cpp .
 * License: BSD
 * Copyright (C) 2016 The Qt Company Ltd.
 * Contact: https://www.qt.io/licensing/
 *
 * This file is part of the Windows main function of the Qt Toolkit.
 *
 * $QT_BEGIN_LICENSE:BSD$
 * Commercial License Usage
 * Licensees holding valid commercial Qt licenses may use this file in
 * accordance with the commercial license agreement provided with the
 * Software or, alternatively, in accordance with the terms contained in
 * a written agreement between you and The Qt Company. For licensing terms
 * and conditions see https://www.qt.io/terms-conditions. For further
 * information use the contact form at https://www.qt.io/contact-us.
 *
 * BSD License Usage
 * Alternatively, you may use this file under the terms of the BSD license
 * as follows:
 *
 * "Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *   * Neither the name of The Qt Company Ltd nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
 */

extern "C" int main(int argc, char *argv[]);
static inline char *wideToMulti(int codePage, const wchar_t *aw)
{
    const int required = WideCharToMultiByte(codePage, 0, aw, -1, NULL, 0, NULL, NULL);
    char *result = new char[required];
    WideCharToMultiByte(codePage, 0, aw, -1, result, required, NULL, NULL);
    return result;
}

extern "C" int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR /*cmdParamarg*/, int /* cmdShow */)
{
    int argc;
    wchar_t **argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argvW)
        return -1;
    char **argv = new char *[argc + 1];
    for (int i = 0; i < argc; ++i)
        argv[i] = wideToMulti(CP_ACP, argvW[i]);
    argv[argc] = Q_NULLPTR;
    LocalFree(argvW);
    const int exitCode = main(argc, argv);
    for (int i = 0; i < argc && argv[i]; ++i)
        delete [] argv[i];
    delete [] argv;
    return exitCode;
}

#endif
