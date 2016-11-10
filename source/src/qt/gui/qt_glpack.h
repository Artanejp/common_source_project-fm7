/*
 * qt_glpack.h
 * (c) 2016 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Define GL texture, vertex etc.
 * History:
 * Nov 10, 2016 : Initial.
 */

#ifndef _QT_COMMON_GLPACK_H
#define _QT_COMMON_GLPACK_H

#include <QString>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLContext>
#include <QGLWidget>
#include <QImage>

#include <QMatrix4x2>
#include <QMatrix4x4>

#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

typedef struct  {
		GLfloat x, y, z;
		GLfloat s, t;
} VertexTexCoord_t;
typedef struct {
		GLfloat x, y;
} VertexLines_t ;

QT_BEGIN_NAMESPACE
class GLScreenPack : public QObject
{
	Q_OBJECT
protected:
	GLuint Texture;
	QOpenGLShaderProgram *program;
	QOpenGLBuffer *vertex_buffer;
	QOpenGLVertexArrayObject *vertex;
	VertexTexCoord_t Vertexs[4];
	int tex_geometry_w, tex_geometry_h, tex_geometry_x, tex_geometry_y;
	int viewport_w, viewport_h, viewport_x, viewport_y;

	bool init_status;
	bool shader_status;
public:
	GLScreenPack(QObject *parent = NULL);
	~GLScreenPack();
	virtual bool initialize(int total_width, int total_height,
							int width, int height,
							const QString &vertex_shader_file, const QString &fragment_shader_file);
	virtual void updateVertex(int viewport_width, int viewport_height,
							  int screen_width, int screen_height,
							  int texture_width, int texture_height,
							  int texture_screen_width, int texture_screen_height,
							  float position_z = -0.9,
							  QOpenGLBuffer::UsagePattern mode = QOpenGLBuffer::DynamicDraw);
	virtual void setNormalVAO(VertexTexCoord_t *tp, int size);
	
	QOpenGLShaderProgram *getShader(void) { return program; }
	QOpenGLBuffer *getVertexBuffer(void) { return vertex_buffer; }
	QOpenGLVertexArrayObject *getVAO(void) { return vertex; }
	VertexTexCoord_t getVertexPos(int pos);
	void setTexture(GLuint id);
	GLuint getTexture();
	
	bool addVertexShaderSrc(const QString &fileName)
	{
		return program->addShaderFromSourceFile(QOpenGLShader::Vertex, fileName);
	}
	bool addFragmentShaderSrc(const QString &fileName)
	{
		return program->addShaderFromSourceFile(QOpenGLShader::Fragment, fileName);
	}

	bool linkProgram(const QString &fileName)
	{
		return program->link();
	}
	QString getShaderLog(void)
	{
		return program->log();
	}
	virtual void bind(void)
	{
		vertex->bind();
		vertex_buffer->bind();
		program->bind();
	}
	virtual void release(void)
	{
		program->release();
		vertex_buffer->release();
		vertex->release();
	}
};
QT_END_NAMESPACE


#endif
