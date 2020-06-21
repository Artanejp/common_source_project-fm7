/*
 * qt_glutil_gl3_0.cpp
 * (c) 2016 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Renderer with OpenGL v3.0 (extend from renderer with OpenGL v2.0).
 * History:
 * Jan 22, 2016 : Initial.
 */

#ifndef _QT_COMMON_GLUTIL_3_0_H
#define _QT_COMMON_GLUTIL_3_0_H

#include <QString>
#include "../gl2/qt_glutil_gl2_0.h"
//#include "../gl/qt_glutil_gl_tmpl.h"

QT_BEGIN_NAMESPACE
class GLScreenPack;
class QOpenGLFunctions_3_0;
class GLDrawClass;
class DLL_PREFIX GLDraw_3_0 : public GLDraw_2_0
{
	Q_OBJECT
private:
	QOpenGLFunctions_3_0 *extfunc;
	float ringing_phase;
protected:
	GLScreenPack *main_pass;
	GLScreenPack *std_pass;
	GLScreenPack *ntsc_pass1;
	GLScreenPack *ntsc_pass2;
	GLScreenPack *bitmap_block;
	GLScreenPack *led_pass;
	GLScreenPack *osd_pass;
	QOpenGLBuffer *led_pass_vbuffer[32];
	QOpenGLVertexArrayObject *led_pass_vao[32];
	QOpenGLBuffer *osd_pass_vbuffer[32];
	QOpenGLVertexArrayObject *osd_pass_vao[32];

	VertexTexCoord_t vertexTmpTexture[4];
	
	QOpenGLShaderProgram *grids_shader;
	QOpenGLBuffer *grids_horizonal_buffer;
	QOpenGLVertexArrayObject *grids_horizonal_vertex;
	QOpenGLBuffer *grids_vertical_buffer;
	QOpenGLVertexArrayObject *grids_vertical_vertex;

	GLuint uTmpTextureID;
	const float rot0[4] =   {1, -0,  0, 1};
	const float rot90[4] =  {0,  1, -1,  0};
	const float rot180[4] = {-1, 0,  0, -1};
	const float rot270[4] = {0, -1,  1, 0};
	
	virtual void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
							  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4);
	virtual bool initGridShaders(const QString vertex_fixed, const QString vertex_rotate, const QString fragment);
	virtual bool initGridVertexObject(QOpenGLBuffer **vbo, QOpenGLVertexArrayObject **vao, int alloc_size);
	virtual void set_texture_vertex(float wmul = 1.0f, float hmul = 1.0f);
	virtual void updateGridsVAO(QOpenGLBuffer *bp,
									QOpenGLVertexArrayObject *vp,
									GLfloat *tp,
									int number);

	virtual void drawGridsMain_3(QOpenGLShaderProgram *prg,
								 QOpenGLBuffer *bp,
								 QOpenGLVertexArrayObject *vp,
								 int number,
								 GLfloat lineWidth = 0.2f,
								 QVector4D color = QVector4D(0.0, 0.0, 0.0, 1.0));
	virtual void resizeGL_Screen(void);
	virtual void initPackedGLObject(GLScreenPack **p,
									int _width, int _height,
									const QString vertex_shader, const QString fragment_shader,
									const QString _name, bool req_float = false,
									bool req_highp = false, bool req_alpha_channel = true);
	virtual void drawGridsHorizonal(void);
	virtual void drawGridsVertical(void);

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
	virtual void drawOsdLeds();
	virtual void drawOsdIcons();
	virtual void drawLedMain(GLScreenPack *obj, int num, QVector4D color);
	virtual void set_led_vertex(int bit);
	virtual void set_osd_vertex(int bit);
public:
	GLDraw_3_0(GLDrawClass *parent, USING_FLAGS *p, CSP_Logger *logger, EMU_TEMPLATE *emu = 0);
	~GLDraw_3_0();
	void drawButtons(void);
	virtual void initGLObjects();
	virtual void initLocalGLObjects(void);
	virtual void initFBO(void);
	//virtual void initBitmapVertex(void);
	
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
