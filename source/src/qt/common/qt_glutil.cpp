/*
 * Qt: OpenGL Utils
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */

#undef _USE_OPENCL
#include "agar_logger.h"
#include "common.h"
#include "emu.h"

#include "osd.h"
#include "qt_gldraw.h"
#include "qt_glutil_gl2_0.h"
#include "qt_glutil_gl3_0.h"

#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP
#include <QGLContext>

//extern const char *cl_render;

void GLDrawClass::update_screen(bitmap_t *p)
{
	//if(tick < (1000 / 75)) tick = 1000 / 75;
	if(p != NULL) {
		this->makeCurrent();
		imgptr = &(p->pImage);
		drawUpdateTexture(p);
		this->doneCurrent();
		this->update();
	}
}

void GLDrawClass::InitContextCL(void)
{
}

void GLDrawClass::initializeGL(void)
{
	int i;
	GLfloat xf, yf, delta;
	/*
	 * GL 拡張の取得 20110907-
	 */
	InitFBO(); // 拡張の有無を調べてからFBOを初期化する。
	InitGLExtensionVars();
}

void GLDrawClass::setChangeBrightness(bool flag)
{
	if(extfunc != NULL) {
		extfunc->setChangeBrightness(flag);
	}
}

void GLDrawClass::setBrightness(GLfloat r, GLfloat g, GLfloat b)
{
	if(extfunc != NULL) {
		extfunc->setBrightness(r, g, b);
	}
}

void GLDrawClass::setSmoosing(bool flag)
{
	if(extfunc != NULL) {
		extfunc->setSmoosing(flag);
	}
}

void GLDrawClass::setDrawGLGridVert(bool flag)
{
	if(extfunc != NULL) {
		extfunc->setDrawGLGridVert(flag);
	}
}

void GLDrawClass::setDrawGLGridHoriz(bool flag)
{
	if(extfunc != NULL) {
		extfunc->setDrawGLGridHoriz(flag);
	}
}

void GLDrawClass::setVirtualVramSize(int width, int height)
{
	if(extfunc != NULL) {
		extfunc->setVirtualVramSize(width, height);
	}
}

// OpenGL状態変数
bool GLDrawClass::QueryGLExtensions(const char *str)
{
	char *ext;
	char *p;
	int i;
	int j;
	int k;
	int l;
	int ll;
	return false;
}

void GLDrawClass::InitGLExtensionVars(void)
{
}


	
void GLDrawClass::InitFBO(void)
{
	int i;
	GLfloat xf, yf, delta;
	QOpenGLContext *glContext = QOpenGLContext::currentContext();
	QOpenGLFunctions_3_0 *funcs_3_0 = glContext->versionFunctions<QOpenGLFunctions_3_0>();
	if((funcs_3_0 != NULL) && (extfunc == NULL)){
		extfunc = new GLDraw_3_0(this);
	}
	
	QOpenGLFunctions_2_0 *funcs_2_0 = glContext->versionFunctions<QOpenGLFunctions_2_0>();
	if((funcs_2_0 != NULL)  && (extfunc == NULL)){
		extfunc = new GLDraw_2_0(this);
	}
	if(extfunc != NULL) {
		extfunc->initGLObjects();
		extfunc->initFBO();
		extfunc->initLocalGLObjects();
	} else {
//str_gl_version = QString::fromUtf8("None");
	}
}

void GLDrawClass::SaveToPixmap(void)
{
	if(save_pixmap_req) {
		if(!filename_screen_pixmap.isEmpty()) {
			QImage snapshot = this->grabFrameBuffer();
			snapshot.save(filename_screen_pixmap);
		}
		save_pixmap_req = false;
		filename_screen_pixmap.clear();
	}
}
