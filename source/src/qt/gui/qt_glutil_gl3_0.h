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

#include "qt_glutil_gl2_0.h"
#include <QOpenGLFunctions_3_0>

class DLL_PREFIX GLDraw_3_0 : public GLDraw_2_0
{
	Q_OBJECT
private:
	QOpenGLFunctions_3_0 *extfunc;
protected:
	VertexTexCoord_t vertexTmpTexture[4];
	QOpenGLShaderProgram *tmp_shader;
	QOpenGLShaderProgram *ntsc_pass1_shader;
	QOpenGLShaderProgram *ntsc_pass2_shader;
	
	QOpenGLShaderProgram *grids_shader;

	
	QOpenGLBuffer *buffer_vertex_tmp_texture;
	QOpenGLVertexArrayObject *vertex_tmp_texture;
	
	QOpenGLBuffer *buffer_vertex_pass1_texture;
	QOpenGLVertexArrayObject *vertex_pass1_texture;
	
	QOpenGLBuffer *buffer_vertex_pass2_texture;
	QOpenGLVertexArrayObject *vertex_pass2_texture;
	
	QOpenGLBuffer *grids_horizonal_buffer;
	QOpenGLVertexArrayObject *grids_horizonal_vertex;
	QOpenGLBuffer *grids_vertical_buffer;
	QOpenGLVertexArrayObject *grids_vertical_vertex;

	GLuint uTmpTextureID;
	GLuint uTmpFrameBuffer;
	GLuint uTmpDepthBuffer;

	GLuint uVramPass1Texture;
	GLuint uVramPass1FrameBuffer;
	GLuint uVramPass1RenderBuffer;
	
	GLuint uVramPass2Texture;
	GLuint uVramPass2FrameBuffer;
	GLuint uVramPass2RenderBuffer;
	
	void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
					  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4);

	void drawGridsHorizonal(void);
	void drawGridsVertical(void);
	void set_texture_vertex(QImage *p, int w_wid, int h_wid,
							int w, int h,
							float wmul = 1.0f, float hmul = 1.0f);
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

	void drawMain(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
				  QOpenGLBuffer *bp,
				  VertexTexCoord_t *vertex_data,
				  GLuint texid,
				  QVector4D color, bool f_smoosing,
				  bool do_chromakey = false,
				  QVector3D chromakey = QVector3D(0.0f, 0.0f, 0.0f));
	void renderToTmpFrameBuffer_nPass(GLuint src_texture,
									  GLuint src_w,
									  GLuint src_h,
									  QOpenGLShaderProgram *shader,
									  GLuint out_texture,
									  GLuint dst_w,
									  GLuint dst_h,
									  QOpenGLBuffer *bv,
									  QOpenGLVertexArrayObject *av,
									  GLuint fb,
									  GLuint rb,
									  bool use_chromakey = false);
public:
	GLDraw_3_0(GLDrawClass *parent, USING_FLAGS *p, EMU *emu = 0);
	~GLDraw_3_0();
	void initGLObjects();
	void initLocalGLObjects(void);
	void uploadMainTexture(QImage *p, bool chromakey);
	void drawScreenTexture(void);
public slots:
	void setBrightness(GLfloat r, GLfloat g, GLfloat b);
	void do_set_texture_size(QImage *p, int w, int h);
};
#endif
