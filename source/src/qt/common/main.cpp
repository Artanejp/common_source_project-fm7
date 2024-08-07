
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
#include <memory>
#include "common.h"
#include "fileio.h"

#include "emu.h"
#include "csp_logger.h"

//#include "./menuclasses.h"
#include "mainwidget_base.h"
#include "qt_main.h"

// emulation core
#include <memory>

// Start to define MainWindow.
extern int   MainLoop(int argc, char *argv[]);

#include <QApplication>
#include <qapplication.h>
extern DLL_PREFIX_I void _resource_init(void);
extern DLL_PREFIX_I void _resource_free(void);
extern DLL_PREFIX_I std::shared_ptr<CSP_Logger> logger_ptr;

void DLL_PREFIX CSP_DebugHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
	QString msg_type;
	bool _nr_line = false;
    switch (type) {
    case QtDebugMsg:
		msg_type = QString::fromUtf8("[Qt:DEBUG]");
        break;
    case QtInfoMsg:
		msg_type = QString::fromUtf8("[Qt:INFO]");
        break;
    case QtWarningMsg:
		msg_type = QString::fromUtf8("[Qt:WARN]");
		_nr_line = true;
        break;
    case QtCriticalMsg:
		msg_type = QString::fromUtf8("[Qt:CRITICAL]");
		_nr_line = true;
        break;
    case QtFatalMsg:
		msg_type = QString::fromUtf8("[Qt:FATAL]");
		_nr_line = true;
		break;
    }
	QString msgString = qFormatLogMessage(type, context, msg);
	QString nmsg_l1 = msg_type;
	QString nmsg_l2 = msg_type;
	std::shared_ptr<CSP_Logger> __logger = logger_ptr;
	
#if 0   
   if(msgString.endsWith(QString::fromUtf8("\n"))) {
	
	msgString = msgString.left(msgString.size() - 1);
   }
   if(msgString.startsWith(QString::fromUtf8("\n"))) {
	
	msgString = msgString.right(msgString.size() - 1);
   }
#endif   
    if(_nr_line) {
	
		nmsg_l2.append(" ");
		nmsg_l2.append(msgString);

		QString tmps_l1 = QString("%1").arg(context.line);
		nmsg_l1.append(" In line ");
		nmsg_l1.append(tmps_l1);
		nmsg_l1.append(" of ");
		nmsg_l1.append(context.file);
		nmsg_l1.append(" (Function: ");
		nmsg_l1.append(context.function);
		nmsg_l1.append(" )");

		if(__logger.get() != nullptr) {
			__logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GUI, nmsg_l1.toLocal8Bit().constData());
			__logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GUI, nmsg_l2.toLocal8Bit().constData());
		} else {
			fprintf(stderr,"%s\n", nmsg_l1.toLocal8Bit().constData());
			fprintf(stderr, "%s\n", nmsg_l2.toLocal8Bit().constData());
		}		
	} else {
		if(__logger.get() != nullptr) {
			__logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GUI, msgString.toLocal8Bit().constData());
		} else {
			fprintf(stderr, "%s\n", msgString.toLocal8Bit().constData());
		}
	}
}

int main(int argc, char *argv[])
{
	int nErrorCode;
	/*
	 * Get current DIR
	 */
/*
 * アプリケーション初期化
 */
	_resource_init();
	logger_ptr.reset();
	
	const char *_p_backtrace = "\n  at line %{line} of %{file} (Thread %{threadid}: function %{function}\nBacktrace:\n %{backtrace depth=15 separator=\"\n \" }";
	QString _s_backtrace = QString::fromUtf8(_p_backtrace);
	QString _s_basicpattern = QString::fromUtf8("%{message}");
	QString _msgpattern;
	_msgpattern = QString::fromUtf8("");
	_msgpattern = _msgpattern + QString::fromUtf8("%{if-debug}[%{type}]") + _s_basicpattern + QString::fromUtf8("%{endif} ");
	_msgpattern = _msgpattern + QString::fromUtf8("%{if-info}[%{type}]") + _s_basicpattern + QString::fromUtf8("%{endif} ");
	_msgpattern = _msgpattern + QString::fromUtf8("%{if-warning}[%{type}]") + _s_basicpattern + QString::fromUtf8("%{endif} ");
	
	//_msgpattern = _msgpattern + QString::fromUtf8("%{if-warning} [%{type}]") + _s_basicpattern + _s_backtrace + QString::fromUtf8("%{endif} ");
	_msgpattern = _msgpattern + QString::fromUtf8("%{if-fatal}[%{type}]") + _s_basicpattern + _s_backtrace + QString::fromUtf8("%{endif} ");
	_msgpattern = _msgpattern + QString::fromUtf8("%{if-critical}[%{type}]") + _s_basicpattern + _s_backtrace + QString::fromUtf8("%{endif} ");
	
	qSetMessagePattern(_msgpattern);
	qInstallMessageHandler(CSP_DebugHandler);
	nErrorCode = MainLoop(argc, argv);
	_resource_free();
	
	return nErrorCode;
}

#if defined(Q_OS_WIN) 
//extern DLL_PREFIX_I int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR /*cmdParamarg*/, int /* cmdShow */);
#if 1
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

//extern "C" DLL_PREFIX int main(int argc, char *argv[]);
static inline char *wideToMulti(int codePage, const wchar_t *aw)
{
    const int required = WideCharToMultiByte(codePage, 0, aw, -1, NULL, 0, NULL, NULL);
    char *result = new char[required];
    WideCharToMultiByte(codePage, 0, aw, -1, result, required, NULL, NULL);
    return result;
}

extern "C" DLL_PREFIX int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR /*cmdParamarg*/, int /* cmdShow */)
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
#endif
