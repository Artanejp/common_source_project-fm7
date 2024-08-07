/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	History : 2015.11.10 Split from qt_main.h
	[ win32 main ] -> [ Qt main ] -> [Emu Thread]
*/
#ifndef _CSP_QT_EMU_THREAD_H
#define _CSP_QT_EMU_THREAD_H

#include <QThread>
#include <QQueue>
#include <QString>
#include <QElapsedTimer>
//#include <QMutexLocker>

//#if QT_VERSION >= 0x051400
//#include <QRecursiveMutex>
//#else
//#include <QMutex>
//#endif

#include "common.h"
#include "fileio.h"
#include "emu.h"
#include "vm.h"
#include "menuclasses.h"
#include "mainwidget.h"
#include "config.h"
#include "emu_thread_tmpl.h"

#ifndef MAX_HISTORY
#define MAX_HISTORY 8
#endif

class META_MainWindow;
class EMU;
class QWaitCondition;
class USING_FLAGS;
QT_BEGIN_NAMESPACE

class EmuThreadClass : public EmuThreadClassBase {
	Q_OBJECT
private:
	int64_t interval;
	int64_t sleep_period;
	int run_frames;
	bool first;
	// LED
protected:
	const _TCHAR *get_device_name(void) override;

public:
	EmuThreadClass(Ui_MainWindowBase *rootWindow, std::shared_ptr<USING_FLAGS> p, QObject *parent = 0);
	~EmuThreadClass();
	void run() override; 
					 
public slots:
	
	void do_initialize();
	void doWork();
signals:
	
};

QT_END_NAMESPACE

#endif
