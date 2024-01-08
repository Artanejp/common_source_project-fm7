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
	virtual void initButtons(void) override;

	virtual void initBitmapVAO(void) override;
	virtual void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
							  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4) override;
	
	virtual void resizeGL_Screen(void)  override;
	virtual void drawGridsHorizonal(void) override;
	virtual void drawGridsVertical(void) override;
	
	void resizeGL_SetVertexs(void) override;
	
	void drawGridsMain(GLfloat *tp,
					   int number,
					   GLfloat lineWidth = 0.2f,
					   QVector4D color = QVector4D(0.0f, 0.0f, 0.0f, 1.0f)) override;
	void drawButtons() override;
	void drawBitmapTexture(void) override;
	virtual void drawOsdLeds() override;
	virtual void drawOsdIcons() override;
	virtual void set_osd_vertex(int xbit) override;
	virtual void set_led_vertex(int xbit) override;

public:
	GLDraw_2_0(GLDrawClass *parent, std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, EMU_TEMPLATE *emu = 0);
	~GLDraw_2_0();

	virtual void initGLObjects() override;
	virtual void initFBO(void) override;
	virtual void initLocalGLObjects(void) override;
	virtual void initOsdObjects(void) override;

	virtual void uploadMainTexture(QImage *p, bool chromakey, bool was_mapped) override;

	virtual void drawScreenTexture(void) override;
	void drawGrids(void) override;

	virtual void drawMain(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  VertexTexCoord_t *vertex_data,
						  GLuint texid,
						  QVector4D color, bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f)) override;
	
	virtual void resizeGL_Pre(int width, int height) override;
																			
public slots:
	virtual void do_set_texture_size(QImage *p, int w, int h) override;
	virtual void do_set_horiz_lines(int lines) override;
	virtual void do_set_screen_multiply(float mul) override;
	
	void initializeGL() override;
	virtual void paintGL() override;
	
	void setImgPtr(QImage *p) override;
	void setSmoosing(bool) override;
	void setDrawGLGridVert(bool) override;
	void setDrawGLGridHoriz(bool) override;
	void setVirtualVramSize(int ,int) override;	
	void setChangeBrightness(bool) override;

	void paintGL_OffScreen(int count, int w, int h) override;
	void set_emu_launched(void) override;
	void do_set_led_width(int bitwidth) override;
};
QT_END_NAMESPACE

#endif // _QT_COMMON_GLUTIL_2_0_H
