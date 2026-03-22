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
#include <QElapsedTimer>

#include <chrono>

#include "emu_template.h"
#include "osd_base.h"
#include "vm/vm.h"

#include "qt_main.h"
#include "csp_logger.h"
#include "mainwidget_base.h"
#include "draw_thread.h"
#include "../emu_thread/emu_thread_tmpl.h"
#include "gl2/qt_glutil_gl2_0.h"
#include "config.h"


DrawThreadClass::DrawThreadClass(OSD_BASE *o, std::shared_ptr<CSP_Logger> logger,QObject *parent) : /*QThread(parent) */ QThread(nullptr) {
	MainWindow = (Ui_MainWindowBase *)parent;
	glv = MainWindow->getGraphicsView();
	p_osd = o;
	csp_logger = logger;
	p_config = nullptr;
	using_flags.reset();
	if(p_osd != nullptr) {
		using_flags = p_osd->get_config_flags();
		if(using_flags.get() != nullptr) {
			p_config = using_flags->get_config_ptr();
		}
	}
	screen = QGuiApplication::primaryScreen();

	is_shared_glcontext = false;
	glContext = NULL;
	draw_screen_buffer = NULL;
	if(p_osd != NULL) {
		p_osd->set_glview(glv);
		//printf("OSD/Context sharing succeeded.ADDR=%08x GLES=%s\n", glContext, (glContext->isOpenGLES()) ? "YES" : "NO");
	}
	do_change_refresh_rate(screen->refreshRate());

	connect(screen, SIGNAL(refreshRateChanged(qreal)), this, SLOT(do_change_refresh_rate(qreal)));
	connect(this, SIGNAL(sig_update_screen(void *, bool)), glv, SLOT(update_screen(void *, bool)), Qt::QueuedConnection);

	connect(this, SIGNAL(sig_update_osd()), glv, SLOT(update_osd()), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_push_frames_to_avio(int, int, int)), glv->extfunc, SLOT(paintGL_OffScreen(int, int, int)));
	connect(glv, SIGNAL(frameSwapped()), this, SLOT(do_vsync()));
	//connect(this, SIGNAL(sig_call_draw_screen()), p_osd, SLOT(draw_screen()));
	//connect(this, SIGNAL(sig_call_no_draw_screen()), p_osd, SLOT(no_draw_screen()));
	m_rec_frame_width = 640;
	m_rec_frame_height = 480;
	m_rec_frame_count = -1;
	m_emu_frame_rate = 1000.0 / 30.0;
	m_wait_count = m_emu_frame_rate.load();
	m_wait_refresh = m_emu_frame_rate.load();
	m_update_req = true;
	m_drawreq_from_host = false;
	textureMappingSemaphore = new QSemaphore(0);
	m_mapping_status = false;
	m_mapped_drawn = false;
	m_about_to_quit = false;
	m_ncount = 0;
	m_req_draw = false;
	//tick_timer.start();
}

DrawThreadClass::~DrawThreadClass()
{
	if(textureMappingSemaphore != NULL) {
		while(textureMappingSemaphore->available() <= 0) textureMappingSemaphore->release(1);
		delete textureMappingSemaphore;
	}

}

void DrawThreadClass::run()
{
	//tick_timer.restart();
	do {
		
		int64_t _us = (int)(m_emu_frame_rate.load() * 1000.0 * 2.0);
		if(_us < 100) {
			_us = 100;
		} else if(_us > (200 * 1000)) {
			_us = 200 * 1000;
		}

		std::chrono::microseconds wait_us(_us);
		if(m_worker_locker.try_lock_for(wait_us)) {
			if((m_update_req.load()) && (p_osd != nullptr) && !(m_about_to_quit.load())) {
				std::lock_guard<std::recursive_mutex> locker(m_main_locker);
				p_osd->do_decode_movie(1);
				if(m_req_draw.load()) {
					m_draw_frames = p_osd->draw_screen();
				} else {
					m_draw_frames = p_osd->no_draw_screen();
				}
				m_update_req = false;
				emit sig_draw_frames(m_draw_frames);
			}
		}
		if(!(m_about_to_quit.load())) {
			int64_t sleep_us = (int64_t)((1000.0 * m_emu_frame_rate.load()) / 8.0);
			if(sleep_us < 1000) {
				yieldCurrentThread();
				continue;
			} else if(sleep_us < 2000) {
				sleep_us = 2000;
			} else if(sleep_us >= (50 * 1000)) {
				sleep_us = 50 * 1000;
			}
			usleep(sleep_us);
		}
	} while(!(m_about_to_quit.load()));
	exit(0);
}

void DrawThreadClass::SetEmu(EMU_TEMPLATE *p)
{
	//p_emu = p;
	p_osd = (OSD_BASE*)(p->get_osd());
}

void DrawThreadClass::do_set_frames_per_second(double fps)
{
	std::lock_guard<std::recursive_mutex> locker(m_main_locker);
	double _n = 1000.0 / fps;
	m_emu_frame_rate = _n;
	m_wait_count += ((_n  / 2.0) * 1.0);
}

void DrawThreadClass::do_vsync()
{
	std::lock_guard<std::recursive_mutex> locker(m_main_locker);
	// ToDo: Swapping buffer.
//	if(m_update_req.load()) {
//		do_draw_one_turn(true);
//	}
//	m_update_req = false;
}

void DrawThreadClass::do_exit_draw_thread(void)
{
	csp_logger->debug_log(CSP_LOG_INFO, CSP_LOG_TYPE_GENERAL,
						  "DrawThread : Exit.");
	m_about_to_quit = true;	
	m_worker_locker.unlock();
	//quit();
}

void DrawThreadClass::do_draw_one_turn(bool _req_draw)
{
	bool __mapped = m_mapped_drawn.load();
	if((_req_draw) && (draw_screen_buffer != NULL)) {
		emit sig_update_screen((void *)draw_screen_buffer, __mapped);
	}
	m_mapped_drawn = false;
	if(m_ncount == 0) {
		emit sig_update_osd();
	}
//	m_mapped_drawn = false;
	m_ncount++;
	if(m_ncount >= 8) m_ncount = 0;
	int _rec_frames = m_rec_frame_count.load();

	if(_rec_frames > 0) {
		emit sig_push_frames_to_avio(_rec_frames,
									 m_rec_frame_width.load(), m_rec_frame_height.load());
		m_rec_frame_count = -1;
	}
	
}

void DrawThreadClass::do_set_priority(QThread::Priority prio)
{
	setPriority(prio);
}


void DrawThreadClass::do_change_refresh_rate(qreal rate)
{
	double _n = 1000.0 / (rate * 2.0);
	m_refresh_rate = rate;
	m_wait_refresh = _n;
	m_wait_count += (_n * 1.0);
}


void DrawThreadClass::do_draw(bool flag)
{
	std::lock_guard<std::recursive_mutex> locker(m_main_locker);
	m_update_req = true;
	m_req_draw   = flag;
	m_worker_locker.unlock();
}
// Event handling.
void DrawThreadClass::do_update_screen(void *p, bool is_mapped, bool already_drawn)
{
	std::lock_guard<std::recursive_mutex> locker(m_main_locker);
	draw_screen_buffer = (bitmap_t*)p;
	m_mapped_drawn = is_mapped;
	do_draw_one_turn((draw_screen_buffer != nullptr) ? true : false);
}

void DrawThreadClass::do_req_encueue_video(int count, int width, int height)
{
	m_rec_frame_width = width;
	m_rec_frame_height = height;
	m_rec_frame_count = count;
}
// Note: Mapping vram from draw_thread does'nt work well.
// This feature might be disable. 20180728 K.Ohta.
void DrawThreadClass::req_map_screen_texture()
{
	if(!(m_mapping_status.load())) {
		if(glv->is_ready_to_map_vram_texture()) {
			textureMappingSemaphore->acquire();
			m_mapping_status = true;
		}
	}
}

void DrawThreadClass::req_unmap_screen_texture()
{
	if(m_mapping_status.load()) {
		if(glv->is_ready_to_map_vram_texture()) {
			emit sig_unmap_texture();
			textureMappingSemaphore->acquire();
			m_mapping_status = false;
			//p_osd->do_set_screen_map_texture_address(mapping_pointer, mapping_width, mapping_height);
		}
	}
}
