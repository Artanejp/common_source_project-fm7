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
#include <QSemaphore>
#include <QScreen>
#include <QWaitCondition>

#include <SDL.h>
#include "emu.h"
#include "osd.h"
#include "vm/vm.h"

#include "qt_main.h"
#include "csp_logger.h"
#include "mainwidget_base.h"
#include "draw_thread.h"
#include "gl2/qt_glutil_gl2_0.h"
#include "config.h"


DrawThreadClass::DrawThreadClass(OSD *o, CSP_Logger *logger,QObject *parent) : QThread(parent) {
	MainWindow = (Ui_MainWindowBase *)parent;
	glv = MainWindow->getGraphicsView();
	p_osd = o;
	csp_logger = logger;
	using_flags = NULL;
	if(p_osd != NULL) using_flags = p_osd->get_config_flags();
	screen = QGuiApplication::primaryScreen();
	
	draw_screen_buffer = NULL;
	
	do_change_refresh_rate(screen->refreshRate());
	connect(screen, SIGNAL(refreshRateChanged(qreal)), this, SLOT(do_change_refresh_rate(qreal)));
	connect(this, SIGNAL(sig_update_screen(bitmap_t *)), glv, SLOT(update_screen(bitmap_t *)), Qt::QueuedConnection);
	
	connect(this, SIGNAL(sig_update_osd()), glv, SLOT(update_osd()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_push_frames_to_avio(int, int, int)), glv->extfunc, SLOT(paintGL_OffScreen(int, int, int)));
	connect(this, SIGNAL(sig_map_texture()), glv, SLOT(do_map_vram_texture()));
	connect(this, SIGNAL(sig_unmap_texture()), glv, SLOT(do_unmap_vram_texture()));
	connect(glv,  SIGNAL(sig_map_texture_reply(bool, void *, int, int)), this, SLOT(do_recv_texture_map_status(bool, void *, int, int)));
	connect(glv,  SIGNAL(sig_unmap_texture_reply()), this, SLOT(do_recv_texture_unmap_status()));
	
	//connect(this, SIGNAL(sig_call_draw_screen()), p_osd, SLOT(draw_screen()));
	//connect(this, SIGNAL(sig_call_no_draw_screen()), p_osd, SLOT(no_draw_screen()));
	use_separate_thread_draw = true;
	if(using_flags->get_config_ptr() != NULL) {
		use_separate_thread_draw = using_flags->get_config_ptr()->use_separate_thread_draw;
	}
	rec_frame_width = 640;
	rec_frame_height = 480;
	rec_frame_count = -1;
	emu_frame_rate = 1000.0 / 30.0;
	wait_count = emu_frame_rate;
	wait_refresh = emu_frame_rate;
	bDrawReq = true;
	renderSemaphore = new QSemaphore(0);
	textureMappingSemaphore = new QSemaphore(0);
	mapping_status = false;
	mapping_pointer = NULL;
	mapping_width = 0;
	mapping_height = 0;
	mapped_drawn = false;
}

DrawThreadClass::~DrawThreadClass()
{
	if(renderSemaphore != NULL) {
		while(renderSemaphore->available() <= 0) renderSemaphore->release(1);
		delete renderSemaphore;
	}
	if(textureMappingSemaphore != NULL) {
		while(textureMappingSemaphore->available() <= 0) textureMappingSemaphore->release(1);
		delete textureMappingSemaphore;
	}

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

void DrawThreadClass::doDrawMain(bool flag)
{
	req_map_screen_texture();
	p_osd->do_decode_movie(1);
	if(flag) {
		draw_frames = p_osd->draw_screen();
	} else {
		draw_frames = p_osd->no_draw_screen();
	}
	req_unmap_screen_texture();

	emit sig_draw_frames(draw_frames);
}
void DrawThreadClass::doDraw(bool flag)
{
	bRecentRenderStatus = flag;
	if(!use_separate_thread_draw) {
		doDrawMain(flag);
	} else {
		if(renderSemaphore != NULL) renderSemaphore->release(1);
	}
}

void DrawThreadClass::doExit(void)
{
	//csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL,
	//					  "DrawThread : Exit.");
	bRunThread = false;
	if(renderSemaphore != NULL) {
		while(renderSemaphore->available() <= 0) renderSemaphore->release(1);
	}
}

void DrawThreadClass::do_draw_one_turn(bool _req_draw)
{
	if((mapped_drawn) && (_req_draw)) {
		emit sig_update_screen(NULL);
	} else if((_req_draw) && (draw_screen_buffer != NULL)) {
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
	mapped_drawn = false;
}

void DrawThreadClass::doWork(const QString &param)
{
	ncount = 0;
	bRunThread = true;
	double _rate = 1000.0 / 30.0;
	bDrawReq = false;
	if(renderSemaphore == NULL) goto __exit;
	do {
		_rate = (wait_refresh < emu_frame_rate) ? emu_frame_rate : wait_refresh;
		if(_rate < 2.0) {
			wait_factor = 2.0;
		} else {
			wait_factor = (int)_rate - 1;
		}
		if(renderSemaphore->tryAcquire(1, wait_factor)) { // Success
			if(!bRunThread) break;
			volatile bool _b = bRecentRenderStatus;
			bRecentRenderStatus = false;
			doDrawMain(_b);
		}
		if(!bRunThread) break;
		volatile bool _d = bDrawReq;
		if(draw_screen_buffer == NULL) _d = false;
		if((_d) && (draw_screen_buffer != NULL)) bDrawReq = false;
		do_draw_one_turn(_d);
	} while(bRunThread);
__exit:
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

void DrawThreadClass::req_map_screen_texture()
{
	mapping_status = false;
	mapping_pointer = NULL;
	mapping_width = 0;
	mapping_height = 0;
	if(glv->is_ready_to_map_vram_texture()) {
		emit sig_map_texture();
		textureMappingSemaphore->acquire();
	}
}

void DrawThreadClass::req_unmap_screen_texture()
{
	if(mapping_status) {
		if(glv->is_ready_to_map_vram_texture()) {
			emit sig_unmap_texture();
			textureMappingSemaphore->acquire();
		}
	}
}

void DrawThreadClass::do_recv_texture_map_status(bool f, void *p, int width, int height)
{
	mapping_status = f;
	mapping_pointer = (scrntype_t *)p;
	mapping_width = width;
	mapping_height = height;
	p_osd->do_set_screen_map_texture_address(mapping_pointer, mapping_width, mapping_height);
	if(mapping_status) {
		mapped_drawn = true;
	}
	textureMappingSemaphore->release(1);
}

void DrawThreadClass::do_recv_texture_unmap_status(void)
{
	mapping_status = false;
	mapping_pointer = NULL;
	mapping_width = 0;
	mapping_height = 0;
	p_osd->do_set_screen_map_texture_address(mapping_pointer, mapping_width, mapping_height);
	textureMappingSemaphore->release(1);
}
