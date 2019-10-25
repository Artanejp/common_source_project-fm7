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
#include <QGLWidget>

#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>

#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QStringList>

typedef struct  {
		GLfloat x, y, z;
		GLfloat s, t;
} VertexTexCoord_t;
typedef struct {
		GLfloat x, y;
} VertexLines_t ;

QT_BEGIN_NAMESPACE
class QOpenGLVertexArrayObject;
class QOpenGLShaderProgram;
class QOpenGLBuffer;
class QOpenGLContext;
class QOpenGLTexture;
class GLScreenPack : public QObject
{
	Q_OBJECT
protected:
	GLuint Texture;
	GLuint frame_buffer_num;
	QOpenGLShaderProgram *program;
	QOpenGLBuffer *vertex_buffer;
	QOpenGLVertexArrayObject *vertex;
	QOpenGLFramebufferObjectFormat fbo_format;
	
	VertexTexCoord_t Vertexs[4];
	int tex_geometry_w, tex_geometry_h, tex_geometry_x, tex_geometry_y;
	int viewport_w, viewport_h, viewport_x, viewport_y;

	bool init_status;
	bool shader_status;
	bool texture_is_float;
	bool texture_is_high_presicion;
	bool need_alpha_channel;
	
	bool has_extension_texture_float;
	bool has_extension_texture_half_float;
	bool has_extension_fragment_high_precision;
	
	QString obj_name;
	QStringList log_str;
	void genBuffer(int width, int height);
	void push_log(QString s) { log_str.append(s); }
	void push_log(const char *s) { log_str.append(QString::fromUtf8(s)); }
public:
	GLScreenPack(int _width, int _height, QString _name = QString::fromUtf8(""), QObject *parent = NULL,
				 bool is_float = false, bool req_high_presicion = false, bool need_alpha = true);
	~GLScreenPack();
	virtual bool initialize(int total_width, int total_height,
							const QString &vertex_shader_file, const QString &fragment_shader_file,
							int width = -1, int height = -1);
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
	GLuint getFrameBufferNum(void) { return frame_buffer_num; }
	VertexTexCoord_t getVertexPos(int pos);
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
		QString s = program->log();
		//QStringList slist = s.split('\n');
		QString ostr = QString::fromUtf8("");
		ostr = s;
		//for(int i = 0; i < slist.size(); i++) {
		//	ostr = ostr + QString("%1: %2\n").arg(i + 1, 5).arg(slist.at(i));
		//}
		return ostr;
	}
	
	QString getGLLog(void)
	{
		QString s;
		QString rets;
		//QString objname = this->objectName();
		s = QString::fromUtf8("");
		rets = QString::fromUtf8("");
		for(int i = 0; i < log_str.size(); i++) {
			s = log_str.at(i);
			rets = rets + QString::fromUtf8("[") + obj_name + QString::fromUtf8("] ");
			//rets = rets + QString("%1: ").arg(i + 1, 5);
			rets = rets + s + QString::fromUtf8(" \n");
		}
		return rets;
	}
	void clearGLLog(void) { log_str.clear(); }
	GLuint getTexture(void)
	{
		return Texture;
	}
	GLuint setTexture(GLuint id)
	{
		Texture = id;
		return id;
	}
	int getViewportWidth()
	{
		return viewport_w;
	}
	int getViewportHeight()
	{
		return viewport_h;
	}
	int getTextureWidth()
	{
		return tex_geometry_w;
	}
	int getTextureHeight()
	{
		return tex_geometry_h;
	}
	virtual void bind(void)
	{
		if(frame_buffer_num != 0) {
			QOpenGLContext *context = QOpenGLContext::currentContext();
			QOpenGLFunctions _fn(context);
			_fn.glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_num);
			_fn.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Texture, 0);
		}
		vertex->bind();
		vertex_buffer->bind();
		program->bind();
	}
	virtual void release(void)
	{
		program->release();
		vertex_buffer->release();
		vertex->release();
		{
			QOpenGLContext *context = QOpenGLContext::currentContext();
			QOpenGLFunctions _fn(context);
			_fn.glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
			_fn.glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}
};
QT_END_NAMESPACE


#endif
