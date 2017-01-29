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

#if defined(_WINDOWS) || defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
#include <GL/wglext.h>
#endif

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

QString GLDrawClass::logGLString(bool getExtensions)
{
	QString s;
	s.clear();

	const GLubyte *(*glGetString)(GLenum) = NULL;
	QOpenGLContext *glContext = QOpenGLContext::currentContext();

	glGetString = (const GLubyte *(*)(GLenum))glContext->getProcAddress(QByteArray("glGetString"));
	if(glGetString != NULL) {
		s.append(QString::fromUtf8("\nSupported OpenGL Vendor: "));
		s.append(QString::fromUtf8((const char *)glGetString(GL_VENDOR)));
		s.append(QString::fromUtf8("\nSupported OpenGL Version: "));
		s.append(QString::fromUtf8((const char *)glGetString(GL_VERSION)));
		s.append(QString::fromUtf8("\nSupported OpenGL Shading Language Version: "));
		s.append(QString::fromUtf8((const char *)glGetString(GL_SHADING_LANGUAGE_VERSION)));
		s.append(QString::fromUtf8("\nSupported OpenGL Renderer: "));
		s.append(QString::fromUtf8((const char *)glGetString(GL_RENDERER)));
		if(getExtensions) {
			s.append(QString::fromUtf8("\nSupported OpenGL Extensions: "));
			s.append(QString::fromUtf8((const char *)glGetString(GL_EXTENSIONS)));
		}
	}
	return s;
}

void GLDrawClass::InitFBO(void)
{
	int i;
	GLfloat xf, yf, delta;
	QOpenGLContext *glContext = QOpenGLContext::currentContext();
	int render_type = using_flags->get_config_ptr()->render_platform;
	int _major_version = using_flags->get_config_ptr()->render_major_version;
	int _minor_version = using_flags->get_config_ptr()->render_minor_version;
	QSurfaceFormat _fmt = glContext->format();
	QSurfaceFormat::RenderableType capability = _fmt.renderableType();
#if !defined(Q_OS_WIN)
	QString tmps = logGLString(false);
	csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "%s", tmps.toLocal8Bit().constData());
#endif
	if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_CORE) {
		QPair<int, int> _glversion = _fmt.version();
		if((((_glversion.first == 3) && (_glversion.second >= 2)) || (_glversion.first >= 4)) &&
		   (extfunc == NULL) &&
		   (((_major_version == 3) && (_minor_version >= 2)) || (_major_version >= 4))){
			//extfunc = new GLDraw_3_0(this, using_flags); // ToDo
			if(extfunc != NULL) {
				csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Use OpenGL v3.2(CORE) Renderer");
			}
		}
	}

	if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_MAIN) {
		QPair<int, int> _glversion = _fmt.version();
		if((_glversion.first >= 3) && (_glversion.second >= 0) &&
		   (extfunc == NULL) &&
		   (_major_version >= 3)){
			extfunc = new GLDraw_3_0(this, using_flags);
			if(extfunc != NULL) {
				csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Use OpenGL v3.0 Renderer");
			}
		}
		if(extfunc == NULL) { // Fallback
			if((_major_version >= 3) || ((_major_version == 2) && (_minor_version >= 1)))  {
				csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Try to use fallback: OpenGL 2.0");
			}				
			extfunc = new GLDraw_2_0(this, using_flags);
			if(extfunc != NULL) {
				csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Use OpenGL v2.0 Renderer");
			}
		}
	}
	if(extfunc != NULL) {
		extfunc->initGLObjects();
		extfunc->initFBO();
		extfunc->initLocalGLObjects();
	} else {
		csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "None using OpenGL.Sorry.");
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
