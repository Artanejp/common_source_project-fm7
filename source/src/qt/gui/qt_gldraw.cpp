/*
 * qt_gldraw.cpp
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */


#include "emu.h"

#include <QtGui>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QRect>
//#include <SDL/SDL.h>
#if defined(_WINDOWS) || defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
#include <GL/gl.h>
#include <GL/glext.h>
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

#include "agar_logger.h"

extern USING_FLAGS *using_flags;

void GLDrawClass::drawGrids(void)
{
	if(extfunc != NULL) extfunc->drawGrids();
}


void GLDrawClass::drawUpdateTexture(bitmap_t *p)
{
	//p_emu->lock_vm();
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
	//p_emu->unlock_vm();
}

void GLDrawClass::updateBitmap(QImage *p)
{
	if(using_flags->is_use_one_board_computer()) {
		if(extfunc != NULL) extfunc->updateBitmap(p);
	}
}

void GLDrawClass::resizeGL(int width, int height)
{
	int side = qMin(width, height);
	double ww, hh;
	if(extfunc != NULL) {
		extfunc->resizeGL(width, height);
	} else {
		draw_width = width;
		draw_height = height;
		delay_update = true;
	}
	//do_set_texture_size(imgptr, screen_texture_width, screen_texture_height);
	AGAR_DebugLog(AGAR_LOG_DEBUG, "ResizeGL: %dx%d", width , height);
	emit sig_resize_uibar(width, height);
}


/*
 * "Paint" Event handler
 */

void GLDrawClass::paintGL(void)
{
	int i;

	SaveToPixmap(); // If save requested, then Save to Pixmap.
	if(extfunc != NULL) {
		if(delay_update) {
			extfunc->setVirtualVramSize(vram_width, vram_height);
			extfunc->resizeGL(draw_width, draw_height);
			delay_update = false;
		}
		extfunc->paintGL();
	}
	emit sig_draw_timing(false);
}

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLDrawClass::GLDrawClass(QWidget *parent)
#if defined(_USE_GLAPI_QT5_4)
  : QOpenGLWidget(parent, Qt::Widget)
#else
  : QGLWidget(parent)
#endif
{
	save_pixmap_req = false;
	enable_mouse = true;
	p_emu = NULL;
	filename_screen_pixmap.clear();
	//imgptr = NULL;
	extfunc = NULL;
	vram_width = SCREEN_WIDTH;
	vram_height = SCREEN_HEIGHT;
	draw_width = SCREEN_WIDTH;
	draw_height = SCREEN_HEIGHT;
	delay_update = false;
	is_mouse_enabled = false;
	this->initKeyCode();
}

GLDrawClass::~GLDrawClass()
{
	if(extfunc != NULL) delete extfunc;
//	this->releaseKeyCode();
	emit sig_finished();

}

void GLDrawClass::setEmuPtr(EMU *p)
{
	p_emu = p;
	if(extfunc != NULL) {
		extfunc->setEmuPtr(p);
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

