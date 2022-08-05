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
							  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4);
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
	virtual void resizeGL_Screen(void);
	virtual void initPackedGLObject(GLScreenPack **p,
								int _width, int _height,
								const QString vertex_shader, const QString fragment_shader,
								const QString _name,
									bool req_float = false, bool req_highp = false,
									bool req_alpha_channel = true);

	virtual void drawGridsHorizonal(void);
	virtual void drawGridsVertical(void);
	virtual void drawGrids();

	virtual void drawMain(QOpenGLShaderProgram *prg,
						  QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  GLuint texid,
						  QVector4D color,
						  bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f));
	virtual void drawMain(GLScreenPack *obj,
						  GLuint texid,
						  QVector4D color, bool f_smoosing,
						  bool do_chromakey = false,
						  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f));
	virtual void renderToTmpFrameBuffer_nPass(GLuint src_texture,
										  GLuint src_w,
										  GLuint src_h,
										  GLScreenPack *renderObject,
										  GLuint dst_w,
										  GLuint dst_h,
										  bool use_chromakey = false);
	virtual void drawBitmapTexture(void);
	virtual void drawButtonsMain(int num, bool f_smoosing);
//	virtual void drawOsdLeds();
//	virtual void drawOsdIcons();
//	virtual void drawLedMain(GLScreenPack *obj, int num, QVector4D color);
	virtual void set_led_vertex(int bit);
	virtual void set_osd_vertex(int bit);

	virtual QOpenGLTexture *createMainTexture(QImage *img);
public:
	GLDraw_ES_2(GLDrawClass *parent, std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger, EMU_TEMPLATE *emu = 0);
	~GLDraw_ES_2();
	void drawButtons(void);
	virtual void initGLObjects();
	virtual void initLocalGLObjects(void);
	virtual void initFBO(void);

	//virtual void initBitmapVertex(void);
	virtual void prologueBlending();
	virtual void epilogueBlending();
	virtual void drawPolygon(int vertex_loc, uintptr_t p = 0);
	
	virtual void uploadMainTexture(QImage *p, bool chromakey, bool was_mapped);
	virtual void drawScreenTexture(void);
	virtual void do_set_screen_multiply(float mul);
	virtual void doSetGridsHorizonal(int lines, bool force);
	virtual void doSetGridsVertical(int pixels, bool force);

public slots:
	void setBrightness(GLfloat r, GLfloat g, GLfloat b);
	void do_set_texture_size(QImage *p, int w, int h);
	void do_set_horiz_lines(int lines);
	virtual void paintGL(void);
	virtual void resizeGL(int width, int height);
};
QT_END_NAMESPACE
#endif
