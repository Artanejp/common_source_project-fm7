
#ifndef _QT_COMMON_GLUTIL_3_0_H
#define _QT_COMMON_GLUTIL_3_0_H

#include "qt_glutil_gl2_0.h"
#include <QOpenGLFunctions_3_0>

class GLDraw_3_0 : public GLDraw_2_0
{
	Q_OBJECT
protected:
	QOpenGLFunctions_3_0 *extfunc_3_0;
	VertexTexCoord_t vertexTmpTexture[4];
	QOpenGLShaderProgram *tmp_shader;
	QOpenGLVertexArrayObject *vertex_tmp_texture;
	QOpenGLBuffer *buffer_vertex_tmp_texture;
	
	GLuint uTmpTextureID;
	GLuint uTmpFrameBuffer;
	GLuint uTmpDepthBuffer;
	void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
					  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4);

public:
	GLDraw_3_0(GLDrawClass *parent, EMU *emu = 0);
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
