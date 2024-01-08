/*
 * qt_glutil_gles_2.h
 * (c) 2018 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Renderer with OpenGL ES v2.0 (extend from renderer with OpenGL v2.0).
 * History:
 * May 05, 2018 : Copy from GL v3.0.
 */

#ifndef _QT_COMMON_GLUTIL_ES_2_H
#define _QT_COMMON_GLUTIL_ES_2_H

#include <QString>

#include "../gl/qt_glutil_gl_tmpl.h"
//#include "../gl2/qt_glutil_gl2_0.h"

QT_BEGIN_NAMESPACE
class GLScreenPack;
class CSP_Logger;
class QOpenGLFunctions;
class QOpenGLBuffer;
class QOpenGLVertexArrayObject;
class QOpenGLShaderProgram;
class QOpenGLPixelTransferOptions;

class DLL_PREFIX GLDraw_ES_2 : public GLDraw_Tmpl
{
	Q_OBJECT
private:
	QOpenGLFunctions *extfunc;
protected:
	bool swap_byteorder;
	QOpenGLPixelTransferOptions *TextureTransferParam;
	
	virtual void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
							  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4) override;
	virtual bool initGridShaders(const QString vertex_fixed, const QString vertex_rotate, const QString fragment);
	virtual bool initGridVertexObject(QOpenGLBuffer **vbo, QOpenGLVertexArrayObject **vao, int alloc_size);
	virtual void set_texture_vertex(float wmul = 1.0f, float hmul = 1.0f);
	virtual void updateGridsVAO(QOpenGLBuffer *bp,
									QOpenGLVertexArrayObject *vp,
									GLfloat *tp,
									int number);
	virtual void drawGridsMain(QOpenGLShaderProgram *prg,
								  QOpenGLBuffer *bp,
								  QOpenGLVertexArrayObject *vp,
								  int number,
								  GLfloat lineWidth = 0.2f,
								  QVector4D color = QVector4D(0.0, 0.0, 0.0, 1.0));
	virtual void resizeGL_Screen(void) override;
	virtual void initPackedGLObject(GLScreenPack **p,
								int _width, int _height,
								const QString vertex_shader, const QString fragment_shader,
								const QString _name,
									bool req_float = false, bool req_highp = false,
									bool req_alpha_channel = true);

	virtual void drawGridsHorizonal(void) override;
	virtual void drawGridsVertical(void) override;
	virtual void drawGrids() override;

	virtual void drawMain(QOpenGLShaderProgram *prg,
						  QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  GLuint texid,
						  QVector4D color,
						  bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f)) override;
	virtual void drawMain(GLScreenPack *obj,
						  GLuint texid,
						  QVector4D color, bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f)) override;
	virtual void renderToTmpFrameBuffer_nPass(GLuint src_texture,
										  GLuint src_w,
										  GLuint src_h,
										  GLScreenPack *renderObject,
										  GLuint dst_w,
										  GLuint dst_h,
										  bool use_chromakey = false) override;
	virtual void drawBitmapTexture(void) override;
	virtual void drawButtonsMain(int num, bool f_smoosing) override;
//	virtual void drawOsdLeds();
//	virtual void drawOsdIcons();
//	virtual void drawLedMain(GLScreenPack *obj, int num, QVector4D color);

	virtual QOpenGLTexture *createMainTexture(QImage *img) override;
public:
	GLDraw_ES_2(GLDrawClass *parent, std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, EMU_TEMPLATE *emu = 0);
	~GLDraw_ES_2();
	void drawButtons(void) override;
	virtual void initGLObjects() override;
	virtual void initLocalGLObjects(void) override;
	virtual void initFBO(void) override;

	//virtual void initBitmapVertex(void);
	virtual void prologueBlending() override;
	virtual void epilogueBlending() override;
	virtual void drawPolygon(int vertex_loc, uintptr_t p = 0) override;
	
	virtual void uploadMainTexture(QImage *p, bool chromakey, bool was_mapped) override;
	virtual void drawScreenTexture(void) override;
	virtual void do_set_screen_multiply(float mul) override;
	virtual void doSetGridsHorizonal(int lines, bool force) override;
	virtual void doSetGridsVertical(int pixels, bool force) override;
	virtual void resizeGL_Pre(int width, int height) override;

public slots:
	void do_set_texture_size(QImage *p, int w, int h) override;
	void do_set_horiz_lines(int lines) override;
	virtual void paintGL(void) override;
};
QT_END_NAMESPACE
#endif
