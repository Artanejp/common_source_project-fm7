/*
 * qt_glutil_gl2_0.h
 * (c) 2016 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Renderer with OpenGL v2.0 .
 * History:
 * Jan 21, 2016 : Initial.
 */

#ifndef _QT_COMMON_GLUTIL_2_0_H
#define _QT_COMMON_GLUTIL_2_0_H

#include <QWidget>
#include <QString>
#include <QList>

#include "config.h"
#include "common.h"
#include "qt_glpack.h"

#include "../gl/qt_glutil_gl_tmpl.h"

QT_BEGIN_NAMESPACE
class QEvent;
class GLDrawClass;
class QOpenGLFramebufferObject;
class QOpenGLFramebufferObjectFormat;
class USING_FLAGS;
class CSP_Logger;
class QOpenGLFunctions_2_0;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QOpenGLShaderProgram;
class QOpenGLTexture;
class DLL_PREFIX GLDraw_2_0 : public GLDraw_Tmpl
{
	Q_OBJECT
private:
	QOpenGLFunctions_2_0 *extfunc_2;
protected:
	virtual void initButtons(void);

	virtual void initBitmapVAO(void);
	virtual void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
							  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4);
	
	virtual void resizeGL_Screen(void);
	virtual void drawGridsHorizonal(void);
	virtual void drawGridsVertical(void);
	
	void resizeGL_SetVertexs(void);
	
	void drawGridsMain(GLfloat *tp,
					   int number,
					   GLfloat lineWidth = 0.2f,
					   QVector4D color = QVector4D(0.0f, 0.0f, 0.0f, 1.0f));
	void drawButtons();
	void drawBitmapTexture(void);
	virtual void drawOsdLeds();
	virtual void drawOsdIcons();
	virtual void set_osd_vertex(int xbit);

public:
	GLDraw_2_0(GLDrawClass *parent, std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, EMU_TEMPLATE *emu = 0);
	~GLDraw_2_0();

	virtual void initGLObjects();
	virtual void initFBO(void);
	virtual void initLocalGLObjects(void);
	virtual void initOsdObjects(void);

	virtual void uploadMainTexture(QImage *p, bool chromakey, bool was_mapped);

	virtual void drawScreenTexture(void);
	void drawGrids(void);

	virtual void drawMain(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  VertexTexCoord_t *vertex_data,
						  GLuint texid,
						  QVector4D color, bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f));
public slots:
	virtual void setBrightness(GLfloat r, GLfloat g, GLfloat b);
	virtual void do_set_texture_size(QImage *p, int w, int h);
	virtual void do_set_horiz_lines(int lines);
	virtual void do_set_screen_multiply(float mul);
	
	void initializeGL();
	virtual void paintGL();
	virtual void resizeGL(int width, int height);

	void setImgPtr(QImage *p);
	void setSmoosing(bool);
	void setDrawGLGridVert(bool);
	void setDrawGLGridHoriz(bool);
	void setVirtualVramSize(int ,int);	
	void setChangeBrightness(bool);

	void paintGL_OffScreen(int count, int w, int h);
	void set_emu_launched(void);
	void do_set_led_width(int bitwidth);
};
QT_END_NAMESPACE

#endif // _QT_COMMON_GLUTIL_2_0_H
