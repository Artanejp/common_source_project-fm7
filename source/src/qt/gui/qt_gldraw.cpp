/*
 * qt_gldraw.cpp
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */


//#include "emu.h"

#include <QtGui>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QRect>
//#include <SDL/SDL.h>
#if defined(_WINDOWS) || defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
#include <GL/gl.h>
#include <GL/glext.h>
#include <GL/wglext.h>
#endif
#include <GL/glu.h>


#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP
#include "../osd.h"
#include "qt_gldraw.h"
#include "gl2/qt_glutil_gl2_0.h"

#include "csp_logger.h"
#include "./qt_drawitem.h"
//extern USING_FLAGS *using_flags;

void GLDrawClass::drawGrids(void)
{
	if(extfunc != NULL) extfunc->drawGrids();
}

void GLDrawClass::drawUpdateTexture(void *p, bool was_mapped)
{
	bitmap_t* pp = (bitmap_t*)p;
	if(using_flags->get_osd() == NULL) return;
	QMutexLocker Locker_S(static_cast<OSD_BASE *>(using_flags->get_osd())->screen_mutex);
	if((pp != NULL)) {
		if(extfunc != NULL) {
// Will fix at implemenitin PX7.
			if(using_flags->is_use_one_board_computer() || (using_flags->get_max_button() > 0)) {
				extfunc->uploadMainTexture(&(pp->pImage), true, was_mapped);
			} else {
				extfunc->uploadMainTexture(&(pp->pImage), false, was_mapped);
			}
		}
	}
}

void GLDrawClass::updateBitmap(QImage *p)
{
	if(using_flags->is_use_one_board_computer()) {
		if(extfunc != NULL) extfunc->updateBitmap(p);
	}
}

void GLDrawClass::resizeGL(int width, int height)
{
	if(extfunc != NULL) {
		extfunc->resizeGL(width, height);
	} else {
		draw_width = width;
		draw_height = height;
		delay_update = true;
	}
	//do_set_texture_size(imgptr, screen_texture_width, screen_texture_height);
	csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "ResizeGL: %dx%d", width , height);
	emit sig_resize_uibar(width, height);
	emit sig_resize_osd(width);
}


/*
 * "Paint" Event handler
 */

void GLDrawClass::paintGL(void)
{
	if(extfunc != NULL) {
		if(delay_update) {
			extfunc->setVirtualVramSize(vram_width, vram_height);
			extfunc->resizeGL(draw_width, draw_height);
			delay_update = false;
		}
		//extfunc->paintGL();
	}
	emit sig_draw_timing();
	SaveToPixmap(); // If save requested, then Save to Pixmap.
}

scrntype_t* GLDrawClass::get_screen_buffer(int y)
{
	if(extfunc == NULL) return NULL;
	return extfunc->get_screen_buffer(y);
}

void GLDrawClass::do_set_display_osd(bool onoff)
{
	emit sig_set_display_osd(onoff);
}

void GLDrawClass::do_display_osd_leds(int lednum, bool onoff)
{
	if((lednum >= -1) && (lednum < 32)) {
		emit sig_display_osd_leds(lednum, onoff);
	}
}


#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif


GLDrawClass::GLDrawClass(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent, const QSurfaceFormat &fmt)
	: QOpenGLWidget(parent, Qt::Widget)
{

	this->setFormat(fmt);

	csp_logger = logger;
	save_pixmap_req = false;
	enable_mouse = true;
	p_emu = NULL;
	using_flags = p;
	p_config = p->get_config_ptr();
	
	filename_screen_pixmap.clear();
	//imgptr = NULL;
	extfunc = NULL;
	vram_width = using_flags->get_screen_width();
	vram_height = using_flags->get_screen_height();
	draw_width = using_flags->get_screen_width();
	draw_height = using_flags->get_screen_height();
	draw_item = new CSP_DrawItem(48, 48);
	
	delay_update = false;
	
	emu_launched = false;
	run_vm = true;

	
	this->initKeyCode();
}

GLDrawClass::~GLDrawClass()
{
	if(extfunc != NULL) delete extfunc;
	if(draw_item != NULL) delete draw_item;
//	this->releaseKeyCode();
	emit sig_finished();

}

void GLDrawClass::do_set_led_width(int bitwidth)
{
	if(extfunc != NULL) {
		extfunc->do_set_led_width(bitwidth);
	}
}
void GLDrawClass::set_emu_launched()
{
	emu_launched = true;
	if(extfunc != NULL) {
		extfunc->set_emu_launched();
	}
}

QSize GLDrawClass::minimumSizeHint() const
{
	return QSize(50, 50);
}

QSize GLDrawClass::sizeHint() const
{
	return QSize(400, 400);
}

QSize GLDrawClass::getCanvasSize(void)
{
	return QSize(this->width(), this->height());
}

QSize GLDrawClass::getDrawSize(void)
{
	return QSize(draw_width, draw_height);
}

void GLDrawClass::do_stop_run_vm()
{
	run_vm = false;
}

// Note: Mapping vram from draw_thread does'nt work well.
// This feature might be disable. 20180728 K.Ohta.
scrntype_t *GLDrawClass::do_map_vram_texture(int *r_width, int *r_height)
{
	bool stat = false;
	int w = 0;
	int h = 0;
	void *p = NULL;
	if(extfunc != NULL) {
		if(extfunc->is_ready_to_map_vram_texture()) {
			stat = extfunc->map_vram_texture();
			if(stat) {
				extfunc->get_screen_geometry(&w, &h);
				p = extfunc->get_screen_buffer(0);
			}
			if(p == NULL) {
				w = 0;
				h = 0;
			}
			if(r_width != NULL) *r_width = w;
			if(r_height != NULL) *r_height = h;
			return (scrntype_t*)p;
		}
	}
	if(r_width != NULL) *r_width = 0;
	if(r_height != NULL) *r_height = 0;
	return NULL;
}

void GLDrawClass::do_unmap_vram_texture()
{
	if(extfunc != NULL) {
		if(extfunc->is_ready_to_map_vram_texture()) {
			extfunc->unmap_vram_texture();	
			return;
		}
	}
}

const bool GLDrawClass::is_ready_to_map_vram_texture(void)
{
	if(extfunc == NULL) return false;
	return extfunc->is_ready_to_map_vram_texture();
}
