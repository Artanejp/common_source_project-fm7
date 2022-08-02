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

#include "emu_template.h"
#include "osd_base.h"
#include "vm/vm.h"

#include "qt_main.h"
#include "csp_logger.h"
#include "mainwidget_base.h"
#include "draw_thread.h"
#include "gl2/qt_glutil_gl2_0.h"
#include "config.h"


DrawThreadClass::DrawThreadClass(OSD_BASE *o, std::shared_ptr<CSP_Logger> logger,QObject *parent) : QThread(parent) {
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
	
	//connect(this, SIGNAL(sig_call_draw_screen()), p_osd, SLOT(draw_screen()));
	//connect(this, SIGNAL(sig_call_no_draw_screen()), p_osd, SLOT(no_draw_screen()));
	use_separate_thread_draw = true;
	if(p_config != nullptr) {
		use_separate_thread_draw = p_config->use_separate_thread_draw;
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

void DrawThreadClass::do_start_draw_thread(void)
{
	bool _separate = false;
	if(p_config == nullptr) {
		p_config = using_flags->get_config_ptr();
	}
	if(p_config != nullptr) {
		if(p_config->use_separate_thread_draw) {
			_separate = true;
		}
	}
	if(_separate) start(QThread::HighPriority);
}

void DrawThreadClass::SetEmu(EMU_TEMPLATE *p)
{
	//p_emu = p;
	p_osd = (OSD_BASE*)(p->get_osd());
}

void DrawThreadClass::do_set_frames_per_second(double fps)
{
	double _n = 1000.0 / (fps * 2.0);
	emu_frame_rate = _n;
	wait_count += (_n * 1.0);
}

// Note: Mapping vram from draw_thread does'nt work well.
// This feature might be disable. 20180728 K.Ohta.
void DrawThreadClass::doDrawMain(bool flag)
{
	//req_map_screen_texture();
	if(p_osd == nullptr) return;
	p_osd->do_decode_movie(1);
	if(flag) {
		draw_frames = p_osd->draw_screen();
	} else {
		draw_frames = p_osd->no_draw_screen();
	}
	//req_unmap_screen_texture();
	emit sig_draw_frames(draw_frames);
}
void DrawThreadClass::doDraw(bool flag)
{
	bRecentRenderStatus = flag;
	if(!use_separate_thread_draw) {
		doDrawMain(flag);
	} else {
		if(renderSemaphore != NULL) {
			if(renderSemaphore->available() < 1) {
				renderSemaphore->release(1);
			}
		}
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
	if((_req_draw) && (draw_screen_buffer != NULL)) {
		emit sig_update_screen((void *)draw_screen_buffer, mapped_drawn);
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
	QElapsedTimer tick_timer;
	tick_timer.start();
	quint64 elapsed = (quint64)_rate;
	double drate;
	double vrate;
	bool rendered = false;
	if(renderSemaphore == NULL) {
		QSemaphore *s = new QSemaphore(0);
		if(s == NULL) goto __exit;
		renderSemaphore = s;
	}
	do {
		double __fps = p_osd->vm_frame_rate(); // FPS; 
		vrate = 1.0e3 / __fps; // to Msec
		_rate = (wait_refresh < emu_frame_rate) ? emu_frame_rate : wait_refresh;
		if((vrate * 2.0) > _rate) _rate = vrate * 2.0; 
		drate = (double)elapsed / 1.0e6; // nsec to msec
		wait_factor = (int)nearbyint(_rate);
//		if(_rate >= drate) {
//			wait_factor = (int)nearbyint(_rate - drate) + ;
//		} else {
//			wait_factor = (int)_rate;
//		}
//		msleep(1);
		if(renderSemaphore->tryAcquire(1, wait_factor)) { // Success
			if(!bRunThread) goto __exit;
			volatile bool _b = bRecentRenderStatus;
			bRecentRenderStatus = false;
			doDrawMain(_b);
			rendered = true;
		} else {
			rendered = false;
		}			
		/*printf("RATE:%f VM_RATE:%f ELAPSED:%f WAIT_FACTOR:%d RENDER=%s\n", _rate, vrate, drate,  wait_factor,
		  (rendered) ? "YES" : "NO");*/
		if(!bRunThread) goto __exit;
		volatile bool _d = bDrawReq;
		if(draw_screen_buffer == NULL) _d = false;
		if((_d) && (draw_screen_buffer != NULL)) bDrawReq = false;
		do_draw_one_turn(_d);
		elapsed = tick_timer.nsecsElapsed();
		tick_timer.start();
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

void DrawThreadClass::do_update_screen(void *p, bool is_mapped)
{
	draw_screen_buffer = (bitmap_t*)p;
	bDrawReq = true;
	mapped_drawn = is_mapped;
}
	
void DrawThreadClass::do_req_encueue_video(int count, int width, int height)
{
	rec_frame_width = width;
	rec_frame_height = height;
	rec_frame_count = count;
}
// Note: Mapping vram from draw_thread does'nt work well.
// This feature might be disable. 20180728 K.Ohta.
void DrawThreadClass::req_map_screen_texture()
{
	mapping_status = false;
	mapping_pointer = NULL;
	mapping_width = 0;
	mapping_height = 0;
	if(glv->is_ready_to_map_vram_texture()) {
		textureMappingSemaphore->acquire();
		//mapping_pointer = (scrntype_t *)(glv->do_map_vram_texture(&mapping_width, &mapping_height));
		//if(mapping_pointer == NULL) {
		//	mapping_status = false;
		//	mapping_pointer = NULL;
		//	mapping_width = 0;
		//	mapping_height = 0;
		//} else {
		//	mapping_status = true;
		//}
		//p_osd->do_set_screen_map_texture_address(mapping_pointer, mapping_width, mapping_height);
	}
}

void DrawThreadClass::req_unmap_screen_texture()
{
	if(mapping_status) {
		if(glv->is_ready_to_map_vram_texture()) {
				emit sig_unmap_texture();
				textureMappingSemaphore->acquire();
				
				//glv->do_unmap_vram_texture();
				//mapping_status = false;
				//mapping_pointer = NULL;
				//mapping_width = 0;
				//mapping_height = 0;
				//p_osd->do_set_screen_map_texture_address(mapping_pointer, mapping_width, mapping_height);
		}
	}
}

