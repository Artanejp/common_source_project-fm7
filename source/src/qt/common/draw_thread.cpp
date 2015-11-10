/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	History : 2015.11.10 Split from qt_main.cpp
	[ win32 main ] -> [ Qt main ] -> [Drawing]
*/

#include <Qt>
#include <QApplication>
#include <QImage>

#include <SDL.h>
#include "emu.h"
#include "vm/vm.h"

#include "qt_main.h"
#include "agar_logger.h"

#include "draw_thread.h"

DrawThreadClass::DrawThreadClass(EMU *p, QObject *parent) : QThread(parent) {
	MainWindow = (Ui_MainWindow *)parent;
	p_emu = emu;
}

void DrawThreadClass::doDraw(void)
{
	QImage *p;
	p_emu->LockVM();
	draw_frames = p_emu->draw_screen();
	p = p_emu->getPseudoVramClass(); 
	p_emu->UnlockVM();
	emit sig_update_screen(p);
	emit sig_draw_frames(draw_frames);
}

void DrawThreadClass::doExit(void)
{
	//bRunThread = false;
	AGAR_DebugLog(AGAR_LOG_DEBUG, "DrawThread : Exit.");
	this->exit(0);
}

void DrawThreadClass::doWork(const QString &param)
{
}


