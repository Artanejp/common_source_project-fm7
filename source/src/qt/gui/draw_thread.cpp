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
#include "csp_logger.h"
#include "mainwidget_base.h"
#include "draw_thread.h"
#include "gl2/qt_glutil_gl2_0.h"

DrawThreadClass::DrawThreadClass(OSD *o, CSP_Logger *logger,QObject *parent) : QThread(parent) {
	MainWindow = (Ui_MainWindowBase *)parent;
	glv = MainWindow->getGraphicsView();
	p_osd = o;
	csp_logger = logger;
	screen = QGuiApplication::primaryScreen();
	
	draw_screen_buffer = NULL;
	
	do_change_refresh_rate(screen->refreshRate());
	connect(screen, SIGNAL(refreshRateChanged(qreal)), this, SLOT(do_change_refresh_rate(qreal)));
	connect(this, SIGNAL(sig_update_screen(bitmap_t *)), glv, SLOT(update_screen(bitmap_t *)), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_update_osd()), glv, SLOT(update_osd()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_push_frames_to_avio(int, int, int)), glv->extfunc, SLOT(paintGL_OffScreen(int, int, int)));
	//connect(this, SIGNAL(sig_call_draw_screen()), p_osd, SLOT(draw_screen()));
	//connect(this, SIGNAL(sig_call_no_draw_screen()), p_osd, SLOT(no_draw_screen()));
	
	rec_frame_width = 640;
	rec_frame_height = 480;
	rec_frame_count = -1;
	emu_frame_rate = 1000.0 / 30.0;
	wait_count = emu_frame_rate;
	wait_refresh = emu_frame_rate;
	bDrawReq = true;
}

DrawThreadClass::~DrawThreadClass()
{
}

void DrawThreadClass::SetEmu(EMU *p)
{
	//p_emu = p;
	p_osd = p->get_osd();
}

void DrawThreadClass::do_set_frames_per_second(double fps)
{
	double _n = 1000.0 / (fps * 2.0);
	emu_frame_rate = _n;
	wait_count += (_n * 1.0);
}

void DrawThreadClass::doDraw(bool flag)
{
	p_osd->do_decode_movie(1);
	if(flag) {
		draw_frames = p_osd->draw_screen();
	} else {
		draw_frames = p_osd->no_draw_screen();
	}
	emit sig_draw_frames(draw_frames);
}

void DrawThreadClass::doExit(void)
{
	//csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL,
	//					  "DrawThread : Exit.");
	bRunThread = false;
}

void DrawThreadClass::do_draw_one_turn(bool _req_draw)
{
	if((_req_draw) && (draw_screen_buffer != NULL)) {
		emit sig_update_screen(draw_screen_buffer);
	} else {
		if(ncount == 0) emit sig_update_osd();
	}
	ncount++;
	if(ncount >= 8) ncount = 0; 
	if(rec_frame_count > 0) {
		emit sig_push_frames_to_avio(rec_frame_count,
									 rec_frame_width, rec_frame_height);
		rec_frame_count = -1;
	}
}

void DrawThreadClass::doWork(const QString &param)
{
	ncount = 0;
	bRunThread = true;
	double _rate = 1000.0 / 30.0;
	bDrawReq = false;
	do {
		_rate = (wait_refresh < emu_frame_rate) ? emu_frame_rate : wait_refresh;
		do_draw_one_turn(bDrawReq);
		if((bDrawReq) && (draw_screen_buffer != NULL)) {
			bDrawReq = false;
		}
		if(wait_count <= 0.0f) {
			wait_count = wait_count + _rate;
		} else if(wait_count < 10.0) {
			msleep(10);
			wait_count = wait_count + _rate - 5.0;
		} else {
			wait_factor = (int)wait_count;
			msleep(wait_factor);
			wait_count -= (qreal)wait_factor;
		}
	} while(bRunThread);
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL,
						  "DrawThread : Exit.");
	this->exit(0);
}

void DrawThreadClass::do_change_refresh_rate(qreal rate)
{
	refresh_rate = rate;	
	wait_refresh = 1000.0 / (refresh_rate * 2.0);
	wait_count += (wait_refresh * 1.0);
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

