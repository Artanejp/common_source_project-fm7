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
#include <QGuiApplication>

#include <SDL.h>
#include "emu.h"
#include "vm/vm.h"

#include "qt_main.h"
#include "agar_logger.h"
#include "mainwidget.h"

#include "draw_thread.h"

DrawThreadClass::DrawThreadClass(EMU *p, QObject *parent) : QThread(parent) {
	MainWindow = (Ui_MainWindow *)parent;
	glv = MainWindow->getGraphicsView();
	p_emu = emu;
	screen = QGuiApplication::primaryScreen();
	
	draw_screen_buffer = NULL;
	
	do_change_refresh_rate(screen->refreshRate());
	connect(screen, SIGNAL(refreshRateChanged(qreal)), this, SLOT(do_change_refresh_rate(qreal)));
	connect(this, SIGNAL(sig_update_screen(screen_buffer_t *)), glv, SLOT(update_screen(screen_buffer_t *)));
	bDrawReq = false;
}

DrawThreadClass::~DrawThreadClass()
{
}

void DrawThreadClass::doDraw(bool flag)
{
	QImage *p;
	if(flag) {
		emit sig_draw_timing(true);
		draw_frames = p_emu->draw_screen();
	} else {
		draw_frames = 1;
	}
	emit sig_draw_frames(draw_frames);
}

void DrawThreadClass::doExit(void)
{
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "DrawThread : Exit.");
	bRunThread = false;
	//this->exit(0);
}

void DrawThreadClass::doWork(const QString &param)
{
	bRunThread = true;
	do {
		if(bDrawReq) {
			if(draw_screen_buffer != NULL) {
				bDrawReq = false;
				emit sig_update_screen(draw_screen_buffer);
			}
		}
		if(wait_count < 1.0f) {
			msleep(1);
			wait_count = wait_count + wait_refresh - 1.0f;
		} else {
			wait_factor = (int)wait_count;
			msleep(wait_factor);
			wait_count -= (qreal)wait_factor;
		}
	} while(bRunThread);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "DrawThread : Exit.");
	this->exit(0);
}

void DrawThreadClass::do_change_refresh_rate(qreal rate)
{
	refresh_rate = rate;	
	wait_refresh = 1000.0f / (refresh_rate * 1.0);
	wait_factor = (int)wait_refresh;
	wait_count = wait_refresh * 1.0;
}

void DrawThreadClass::do_update_screen(screen_buffer_t *p)
{
	draw_screen_buffer = p;
	bDrawReq = true;
}
	
