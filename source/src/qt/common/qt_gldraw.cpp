/*
 * qt_gldraw.cpp
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */


#include "emu.h"

#include <QtGui>
#if defined(MAX_BUTTONS) || defined(ONE_BOARD_MICRO_COMPUTER)
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QRect>
#endif
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

#undef _USE_OPENCL
#ifdef _USE_OPENCL
//# include "agar_glcl.h"
#endif


#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP
#include "qt_gldraw.h"
#include "qt_glutil_gl2_0.h"

#include "agar_logger.h"

#ifdef _USE_OPENCL
extern class GLCLDraw *cldraw;
extern void InitContextCL(void);
#endif


void GLDrawClass::drawGrids(void)
{
	if(extfunc != NULL) extfunc->drawGrids();
}


#if 0 // v3.0
#endif
void GLDrawClass::drawUpdateTexture(bitmap_t *p)
{
	//p_emu->lock_vm();
	if((p != NULL)) {
		if(extfunc != NULL) {
// Will fix at implemenitin PX7.
# if defined(ONE_BOARD_MICRO_COMPUTER) || defined(MAX_BUTTONS)
			extfunc->uploadMainTexture(&(p->pImage), true);	
# else
			extfunc->uploadMainTexture(&(p->pImage), false);	
# endif
		}
	}
	//p_emu->unlock_vm();
}

#if defined(MAX_BUTTONS)
void GLDrawClass::updateButtonTexture(void)
{
	int i;
   	QImage *img;
   	QPainter *painter;
	QColor col;
	QRect rect;
	QPen *pen;
	QFont font = QFont(QString::fromUtf8("Sans"));
	if(button_updated) return;
	col.setRgb(0, 0, 0, 255);
	pen = new QPen(col);
	for(i = 0; i < MAX_BUTTONS; i++) {
		img = new QImage(buttons[i].width * 4, buttons[i].height * 4, QImage::Format_RGB32);
		painter = new QPainter(img);
		painter->setRenderHint(QPainter::Antialiasing, true);
		col.setRgb(255, 255, 255, 255);
		if(strlen(buttons[i].caption) <= 3) {
			font.setPixelSize((buttons[i].width * 4) / 2); 
		} else {
			font.setPixelSize((buttons[i].width * 4) / 4); 
		}
		painter->fillRect(0, 0, buttons[i].width * 4, buttons[i].height * 4, col);
		painter->setFont(font);
		//painter->setPen(pen);
		rect.setWidth(buttons[i].width * 4);
		rect.setHeight(buttons[i].height * 4);
		rect.setX(0);
		rect.setY(0);
		painter->drawText(rect, Qt::AlignCenter, QString::fromUtf8(buttons[i].caption));
		if(uButtonTextureID[i] != 0) {
	  		this->deleteTexture(uButtonTextureID[i]);
		}
		uButtonTextureID[i] = this->bindTexture(*img);;
		delete painter;
		delete img;
	}
	delete pen;
	button_updated = true;
}
#endif

#if defined(ONE_BOARD_MICRO_COMPUTER)
void GLDrawClass::uploadBitmapTexture(QImage *p)
{

}

void GLDrawClass::updateBitmap(QImage *p)
{
	if(extfunc != NULL) extfunc->updateBitmap(p);
}
#endif

void GLDrawClass::resizeGL(int width, int height)
{
	int side = qMin(width, height);
	double ww, hh;
	if(extfunc != NULL) {
		extfunc->resizeGL(width, height);
	}
	redraw_required = true;
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
#if !defined(USE_MINIMUM_RENDERING)
	redraw_required = true;
#endif
	if(!redraw_required) return;
	if(extfunc != NULL) {
		extfunc->paintGL();
	}
	emit sig_draw_timing(false);
	redraw_required = false;
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
	filename_screen_pixmap.clear();
	
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

