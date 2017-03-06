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
#else
# if !defined(_USE_GLAPI_QT5_4) || !defined(_USE_GLAPI_QT5_1)  
#  include <GL/glx.h>
#  include <GL/glxext.h>
# endif
#endif
#include <GL/glu.h>


#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP
#include "qt_gldraw.h"
#include "qt_glutil_gl2_0.h"

#include "csp_logger.h"
#include "../osd.h"

//extern USING_FLAGS *using_flags;

void GLDrawClass::drawGrids(void)
{
	if(extfunc != NULL) extfunc->drawGrids();
}

void GLDrawClass::drawUpdateTexture(bitmap_t *p)
{
	if(using_flags->get_osd() != NULL)	using_flags->get_osd()->screen_mutex->lock();
	if((p != NULL)) {
		if(extfunc != NULL) {
// Will fix at implemenitin PX7.
			if(using_flags->is_use_one_board_computer() || (using_flags->get_max_button() > 0)) {
				extfunc->uploadMainTexture(&(p->pImage), true);
			} else {
				extfunc->uploadMainTexture(&(p->pImage), false);
			}
		}
	}
	if(using_flags->get_osd() != NULL)	using_flags->get_osd()->screen_mutex->unlock();
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
}


/*
 * "Paint" Event handler
 */

void GLDrawClass::paintGL(void)
{
	SaveToPixmap(); // If save requested, then Save to Pixmap.
	//qWarning("Test");
	if(extfunc != NULL) {
		if(delay_update) {
			extfunc->setVirtualVramSize(vram_width, vram_height);
			extfunc->resizeGL(draw_width, draw_height);
			delay_update = false;
		}
		//extfunc->paintGL();
	}
	emit sig_draw_timing();
}


//void GLDrawClass::paintEvent(QPaintEvent *ev)
//{
//	// Do Nothing.
//	// http://doc.qt.io/qt-5/qopenglwidget.html#Threading
//}

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#if defined(_USE_GLAPI_QT5_4)
GLDrawClass::GLDrawClass(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent, const QSurfaceFormat &fmt)
	: QOpenGLWidget(parent, Qt::Widget)
#else
GLDrawClass::GLDrawClass(USING_FLAGS *p, CSP_Logger *logger, QWidget *parent, const QGLFormat &fmt)
	: QGLWidget(fmt, parent)
#endif
{
#if defined(_USE_GLAPI_QT5_4)
	this->setFormat(fmt);
#endif
	csp_logger = logger;
	save_pixmap_req = false;
	enable_mouse = true;
	p_emu = NULL;
	using_flags = p;
	
	filename_screen_pixmap.clear();
	//imgptr = NULL;
	extfunc = NULL;
	vram_width = using_flags->get_screen_width();
	vram_height = using_flags->get_screen_height();
	draw_width = using_flags->get_screen_width();
	draw_height = using_flags->get_screen_height();
	delay_update = false;
	is_mouse_enabled = false;
	emu_launched = false;
	run_vm = true;
	this->initKeyCode();
}

GLDrawClass::~GLDrawClass()
{
	if(extfunc != NULL) delete extfunc;
//	this->releaseKeyCode();
	emit sig_finished();

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
