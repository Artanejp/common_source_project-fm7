/*
 * Qt: OpenGL Utils
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */

#undef _USE_OPENCL
#include "csp_logger.h"
#include "common.h"
//#include "emu.h"

#include "../osd_types.h"
#include "qt_gldraw.h"
#include "qt_glutil_gl2_0.h"
#include "qt_glutil_gl3_0.h"
#include <QOpenGLFunctions_4_1_Core>
#include <QOpenGLFunctions_3_2_Core>

#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP
#include <QGLContext>
#include <QPair>
#include <QSurfaceFormat>

//extern const char *cl_render;

void GLDrawClass::update_screen(bitmap_t *p)
{
	//if(tick < (1000 / 75)) tick = 1000 / 75;
	if((p != NULL) && (run_vm)) {
		this->makeCurrent();
		//imgptr = &(p->pImage);
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
	} else {
		vram_width = width;
		vram_height = height;
		delay_update = true;
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
	QOpenGLVersionProfile prof;
	
	// Try 4.x
	//prof.setProfile(QSurfaceFormat::CoreProfile);
		
	QOpenGLFunctions_2_0 *funcs_2_0 = glContext->versionFunctions<QOpenGLFunctions_2_0>();
	QSurfaceFormat _fmt = glContext->format();
	QOpenGLFunctions *funcs = glContext->functions();
	
	QPair<int, int> _glversion = _fmt.version();
	if((_fmt.renderableType() & QSurfaceFormat::OpenGL) != 0) {
		if((_glversion.first == 3) && (_glversion.second <= 0) && (extfunc == NULL)){
			extfunc = new GLDraw_3_0(this, using_flags);
			csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Use OpenGL v3.0 Renderer");
		} else
		if((funcs_2_0 != NULL)  && (extfunc == NULL)){
			extfunc = new GLDraw_2_0(this, using_flags);
			csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Use OpenGL v2.0 Renderer");
		}
	}
	if(funcs != NULL) {
		csp_logger->debug_log(CSP_LOG_DEBUG, "Supported OpenGL Vendor  %s", funcs->glGetString(GL_VENDOR));
		csp_logger->debug_log(CSP_LOG_DEBUG, "Supported OpenGL Version %s", funcs->glGetString(GL_VERSION));
		csp_logger->debug_log(CSP_LOG_DEBUG, "Supported OpenGL Shading Language Version %s", funcs->glGetString(GL_SHADING_LANGUAGE_VERSION));
		csp_logger->debug_log(CSP_LOG_DEBUG, "Supported OpenGL Renderer %s", funcs->glGetString(GL_RENDERER));
		//csp_logger->debug_log(CSP_LOG_DEBUG, "Supported OpenGL Extensions %s", funcs->glGetString(GL_EXTENSIONS));
	}		

	if(extfunc != NULL) {
		extfunc->initGLObjects();
		extfunc->initFBO();
		extfunc->initLocalGLObjects();
	} else {
		csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "None using OpenGL.");
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
