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

#include "gl/qt_glutil_gl_tmpl.h"
#include "gl2/qt_glutil_gl2_0.h"
#include "gl3/qt_glutil_gl3_0.h"
#include "gl4_5/qt_glutil_gl4_5.h"
#include "gles2/qt_glutil_gles_2.h"

#include <QOpenGLFunctions_4_5_Core>
#include <QApplication>
#include "./qt_drawitem.h"

#if defined(_WINDOWS) || defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
#include <GL/wglext.h>
#endif

#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP
#include <QOpenGLContext>
#include <QPair>
#include <QSurfaceFormat>

//extern const char *cl_render;

void GLDrawClass::update_screen(void *p, bool was_mapped)
{
	//if(tick < (1000 / 75)) tick = 1000 / 75;
	if((p != NULL) && (run_vm)) {
		this->makeCurrent();
		//imgptr = &(p->pImage);
		drawUpdateTexture(p, was_mapped);
		this->doneCurrent();
		this->update();
	}
}

bool GLDrawClass::is_mapped_buffer(void)
{
	if(extfunc != NULL) {
		return extfunc->is_mapped_buffer();
	}
	return false;
}

GLuint GLDrawClass::get_mapped_buffer_num(int region)
{
	if(extfunc != NULL) {
		return extfunc->get_mapped_buffer_num(region);
	}
	return (GLuint)0;
}


void GLDrawClass::do_update_icon(int icon_type,  int localnum, QPixmap *p)
{
	if(extfunc != NULL) {
		extfunc->uploadIconTexture(p, icon_type, localnum);
	}
}

void GLDrawClass::do_update_icon(int icon_type, int localnum, QString message, QColor bg, QColor fg, QColor fg2, QColor fg3, QColor lg, QColor tg, float pt)
{
	if(draw_item != NULL) {
		QPixmap icon;
		QColor nullColor = QColor(0, 0, 0, 0);
		switch(icon_type) {
		case 0:
			draw_item->clearCanvas(nullColor);
			break;
		case 1: 
			draw_item->drawFloppy5Inch(bg, fg3, fg2, tg, pt, message);
			icon_type = 2; // Workaround
			break;
		case 2:
			draw_item->drawFloppy3_5Inch(bg, fg3, fg2, fg, tg, pt, message);
			break;
		case 3:
			draw_item->drawQuickDisk(bg, fg, fg2, tg, pt, message);
			break;
		case 4:
		case 5:
			draw_item->drawCasetteTape(bg, fg, fg2, fg3, tg, pt, message);
			break;
		case 6:
			draw_item->drawCompactDisc(bg, fg, fg2, fg3, lg, tg, pt, message);
			break;
		case 7:
			draw_item->drawLaserDisc(bg, fg, fg2, lg, tg, pt, message);
			break;
		default:
			break;
		}
		icon = QPixmap::fromImage(*draw_item);
		do_update_icon(icon_type, localnum, &icon);
	}
}

void GLDrawClass::update_osd(void)
{
		this->update();
}

void GLDrawClass::InitContextCL(void)
{
}

void GLDrawClass::initializeGL(void)
{
	/*
	 * GL 拡張の取得 20110907-
	 */
	InitFBO(); // 拡張の有無を調べてからFBOを初期化する。
	InitGLExtensionVars();

	QColor BG = QColor(0, 250, 0, 255);
	QColor TG = QColor(0, 0, 250, 255);
	QColor TG2 = QColor(250, 250, 250, 255);
	QColor LG = QColor(250, 0, 0, 255);
	QColor FG = QColor(255, 255, 255, 255);
	QColor FG2 = QColor(128, 128, 128, 255);
	QColor FG3 = QColor(64, 64, 64, 255);

	do_update_icon(0, 0, QString::fromUtf8(""), BG, FG, FG2, FG3, LG, TG, 12.0f);
	if(using_flags->is_use_laser_disc()) {
		do_update_icon(7, 0, QString::fromUtf8("LD"), BG, FG, FG2, FG3, LG, TG, 12.0f);
	}
	if(using_flags->is_use_compact_disc()) {
		do_update_icon(6, 0, QString::fromUtf8("CD"), BG, FG, FG2, FG3, LG, TG, 12.0f);
	}
	if(using_flags->is_use_hdd()) {
		int drvs = using_flags->get_max_hdd();
		QString ts, tmps;
		for(int i = 0; i < drvs; i++) {
			tmps = QString::fromUtf8("");
			ts.setNum(i);
			tmps = tmps + ts + QString::fromUtf8(":");
			//do_update_icon(8, i, tmps, BG, FG, FG2, FG3, LG, TG, 12.0f); // Dedicate to 3.5/5/8? and startnum.
		}
	}
	if(using_flags->is_use_fd()) {
		int drvs = using_flags->get_max_drive();
		uint32_t drvbit = using_flags->get_floppy_type_bit();
		QString ts, tmps;
		for(int i = 0; i < drvs; i++) {
			tmps = QString::fromUtf8("");
			ts.setNum(i);
			tmps = tmps + ts + QString::fromUtf8(":");
			if((drvbit & (1 << i)) == 0) {
				do_update_icon(1, i, tmps, BG, FG, FG2, FG3, LG, TG2, 12.0f); // Dedicate to 3.5/5/8? and startnum.
			} else {
				do_update_icon(2, i, tmps, BG, FG, FG2, FG3, LG, TG2, 12.0f); // Dedicate to 3.5/5/8? and startnum.
			}
		}
	}
	if(using_flags->is_use_qd()) {
		int drvs = using_flags->get_max_qd();
		QString ts, tmps;
		for(int i = 0; i < drvs; i++) {
			tmps = QString::fromUtf8("");
			ts.setNum(i);
			tmps = tmps + ts + QString::fromUtf8(":");
			do_update_icon(3, i, tmps, BG, FG, FG2, FG3, LG, TG, 12.0f); // Dedicate to 3.5/5/8? and startnum.
		}
	}
	if(using_flags->is_use_tape()) {
		int drvs = using_flags->get_max_tape();
		QColor R_BG  = QColor(0, 0, 255, 255);
		QColor W_BG  = QColor(255, 0, 0, 255);
		QColor C_FG  = QColor(255, 255, 255, 255);
		QColor C_FG2 = QColor(64, 64, 64, 255);
		QColor C_FG3 = QColor(128, 128, 128, 255);
		QColor C_TG  = QColor(0, 255, 0, 255);
		QString ts, tmps;
		for(int i = 0; i < drvs; i++) {
			tmps = QString::fromUtf8("");
			ts.setNum(i + 1);
			tmps = tmps + ts;
			do_update_icon(4, i, tmps, R_BG, C_FG, C_FG2, C_FG3, LG, C_TG, 12.0f);
			do_update_icon(5, i, tmps, W_BG, C_FG, C_FG2, C_FG3, LG, C_TG, 12.0f);
		}
	}	
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
	QSurfaceFormat _fmt = glContext->format();
	if(glContext->isOpenGLES()) {
		s.append(QString::fromUtf8("\nUsing OpenGL ES. "));
	} else { // OpenGL
		s.append(QString::fromUtf8("\nUsing OpenGL. "));
	}

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
	QOpenGLContext *glContext = QOpenGLContext::currentContext();
	//QOpenGLContext *glContext = QOpenGLContext::globalShareContext();
	int render_type = p_config->render_platform;
	int _major_version = p_config->render_major_version;
	int _minor_version = p_config->render_minor_version;
	QSurfaceFormat _fmt = glContext->format();
	//QSurfaceFormat::RenderableType capability = _fmt.renderableType();
#if !defined(Q_OS_WIN)
	QString tmps = logGLString(false);
	csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "%s", tmps.toLocal8Bit().constData());
#endif
	if((render_type == CONFIG_RENDER_PLATFORM_OPENGL_ES) || (glContext->isOpenGLES())){
		QPair<int, int> _glversion = _fmt.version();
		if(_major_version >= 3) {
			if((_glversion.first >= 3) &&
			   (extfunc == NULL)) {
				if(_glversion.second >= 1) {
					extfunc = new GLDraw_ES_2(this, using_flags, csp_logger);
					if(extfunc != NULL) {
						_major_version = 3;
						csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Use OpenGL ES(v3.1) Renderer");
						goto _nr_end;
					}
				}
			}
		}
		if((_glversion.first >= 2) && (_glversion.second >= 0) &&
		   (extfunc == NULL) &&
		   (_major_version >= 2)){
			extfunc = new GLDraw_ES_2(this, using_flags, csp_logger);
			if(extfunc != NULL) {
				_major_version = 2;
				_minor_version = 0;
				csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Use OpenGL ES(v2.0) Renderer");
				goto _nr_end;
			}
		}
	}
	if(_fmt.profile() == QSurfaceFormat::CoreProfile) {
		QPair<int, int> _glversion = _fmt.version();
		if(((_glversion.first >= 5) || ((_glversion.first == 4) && (_glversion.second >= 5))) &&
		   (extfunc == NULL) &&
		  ((_major_version >= 5) || ((_major_version == 4) && (_minor_version >= 3)))){
			extfunc = new GLDraw_4_5(this, using_flags, csp_logger); // ToDo
			if(extfunc != NULL) {
				_major_version = 4;
				_minor_version = 5;
				render_type = CONFIG_RENDER_PLATFORM_OPENGL_CORE;
				csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Use OpenGL v4.5(CORE) Renderer");
				goto _nr_end;
			}
		}
	} else /* if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_MAIN) */ {
		QPair<int, int> _glversion = _fmt.version();
		if((_glversion.first >= 3) && (_glversion.second >= 0) &&
		   (extfunc == NULL) &&
		   (_major_version >= 3)){
			extfunc = new GLDraw_3_0(this, using_flags, csp_logger);
			if(extfunc != NULL) {
				_major_version = 3;
				_minor_version = 0;
				csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Use OpenGL v3.0 Renderer");
				render_type = CONFIG_RENDER_PLATFORM_OPENGL_MAIN;
				goto _nr_end;
			}
		}
		if(extfunc == NULL) { // Fallback
			if((_major_version >= 3) || ((_major_version == 2) && (_minor_version >= 1)))  {
				csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Try to use fallback: OpenGL 2.0");
			}				
			extfunc = new GLDraw_2_0(this, using_flags, csp_logger);
			if(extfunc != NULL) {
				_major_version = 2;
				_minor_version = 0;
				csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "Use OpenGL v2.0 Renderer");
				render_type = CONFIG_RENDER_PLATFORM_OPENGL_MAIN;
				goto _nr_end;
			}
		}
	}
_nr_end:
	if(extfunc != NULL) {
		if((render_type != CONFIG_RENDER_PLATFORM_OPENGL_MAIN) &&
		   (render_type != CONFIG_RENDER_PLATFORM_OPENGL_CORE) &&
		   (render_type != CONFIG_RENDER_PLATFORM_OPENGL_ES)) {
			render_string = QString::fromUtf8("Emu:  Not OpenGL");
		} else {
			QString _head;
			if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_ES) {
				_head = QString::fromUtf8("OpenGL ES v");
			} else if(render_type == CONFIG_RENDER_PLATFORM_OPENGL_CORE) {
				_head = QString::fromUtf8("OpenGL (Core) v");
			} else {
				_head = QString::fromUtf8("OpenGL (Main) v");
			}				
			render_string = QString::fromUtf8("Emu:&nbsp;&nbsp;") + _head + QString::number(_major_version) +
				QString::fromUtf8(".") + QString::number(_minor_version) + QString::fromUtf8("<BR>");
		}
		extfunc->initGLObjects();
		extfunc->initFBO();
		extfunc->initLocalGLObjects();
		connect(this, SIGNAL(sig_draw_timing()), extfunc, SLOT(paintGL()));
		connect(this, SIGNAL(sig_set_display_osd(bool)), extfunc, SLOT(do_set_display_osd(bool)));
		connect(this, SIGNAL(sig_display_osd_leds(int, bool)), extfunc, SLOT(do_display_osd_leds(int, bool)));
	} else {
		csp_logger->debug_log(CSP_LOG_DEBUG, CSP_LOG_TYPE_GENERAL, "None using OpenGL.Sorry.");
	}
}

void GLDrawClass::SaveToPixmap(void)
{
	if(save_pixmap_req) {
		if(!filename_screen_pixmap.isEmpty()) {
			//QImage snapshot = QImage(this->size(), QImage::Format_ARGB32_Premultiplied);
			//glReadPixels(0, 0, this->width(), this->height(), GL_RGBA, GL_UNSIGNED_BYTE, snapshot.bits());
			QImage snapshot = this->grabFramebuffer().convertToFormat(QImage::Format_RGB32);
			snapshot.save(filename_screen_pixmap);
		}
		save_pixmap_req = false;
		filename_screen_pixmap.clear();
	}
}

void GLDrawClass::do_move_mouse_to_center(void)
{
	QCursor cursor;
	QPoint pos;
	cursor = this->cursor();
	pos.setX(this->width() / 2);
	pos.setY(this->height() / 2);
	cursor.setPos(this->mapToGlobal(pos));
}

void GLDrawClass::do_enable_mouse(void)
{
	do_move_mouse_to_center();
	QApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
	//mouse_shape = cursor.shape();
	//cursor.setShape(Qt::BlankCursor);
	this->setMouseTracking(true);
}

void GLDrawClass::do_disable_mouse(void)
{
	if(QApplication::overrideCursor() != NULL) QApplication::restoreOverrideCursor();
	setMouseTracking(false);
}
