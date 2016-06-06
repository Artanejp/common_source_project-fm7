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
#include "osd.h"
#include "vm/vm.h"

#include "qt_main.h"
#include "agar_logger.h"
#include "mainwidget_base.h"
#include "draw_thread.h"
#include "qt_glutil_gl2_0.h"

DrawThreadClass::DrawThreadClass(EMU *p, OSD *o, QObject *parent) : QThread(parent) {
	MainWindow = (Ui_MainWindowBase *)parent;
	glv = MainWindow->getGraphicsView();
	p_emu = emu;
	p_osd = o;
	screen = QGuiApplication::primaryScreen();
	
	draw_screen_buffer = NULL;
	
	do_change_refresh_rate(screen->refreshRate());
	connect(screen, SIGNAL(refreshRateChanged(qreal)), this, SLOT(do_change_refresh_rate(qreal)));
	connect(this, SIGNAL(sig_update_screen(bitmap_t *)), glv, SLOT(update_screen(bitmap_t *)));
	connect(this, SIGNAL(sig_push_frames_to_avio(int, int, int)), glv->extfunc, SLOT(paintGL_OffScreen(int, int, int)));
	rec_frame_width = 640;
	rec_frame_height = 480;
	rec_frame_count = -1;

	bDrawReq = false;
}

DrawThreadClass::~DrawThreadClass()
{
}

void DrawThreadClass::SetEmu(EMU *p)
{
		p_emu = p;
		p_osd = p->get_osd();
}


void DrawThreadClass::doDraw(bool flag)
{
	QImage *p;
	if(flag) {
		emit sig_draw_timing(true);
		draw_frames = p_osd->draw_screen();
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
		if(rec_frame_count > 0) {
			emit sig_push_frames_to_avio(rec_frame_count,
										 rec_frame_width, rec_frame_height);
			rec_frame_count = -1;
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
	wait_refresh = 1000.0f / (refresh_rate * 2.0);
	wait_count = wait_refresh * 1.0;
}

void DrawThreadClass::do_update_screen(bitmap_t *p)
{
	draw_screen_buffer = p;
	bDrawReq = true;
}
	
void DrawThreadClass::do_req_encueue_video(int count, int width, int height)
{
	rec_frame_width = width;
	rec_frame_height = height;
	rec_frame_count = count;
}
