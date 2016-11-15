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

#include <QtGui>
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QRect>
#include <QGLWidget>
#include <QImage>
#include <QOpenGLFunctions_2_0>
#include <QTimer>

#include "common.h"

#include "qt_glpack.h"

class EMU;
class QEvent;
class GLDrawClass;
class QOpenGLFramebufferObject;
class QOpenGLFramebufferObjectFormat;
class USING_FLAGS;

class DLL_PREFIX GLDraw_2_0 : public QObject
{
	Q_OBJECT
private:
	QOpenGLFunctions_2_0 *extfunc_2;
protected:
	GLDrawClass *p_wid;
	USING_FLAGS *using_flags;
	EMU *p_emu;
	QImage *imgptr;
	bool smoosing;
	bool gl_grid_horiz;
	bool gl_grid_vert;
	
	int  vert_lines;
	int  horiz_pixels;
	GLfloat *glVertGrids;
	GLfloat *glHorizGrids;
	float screen_multiply;

	float screen_width;
	float screen_height;
	
	int screen_texture_width;
	int screen_texture_width_old;
	int screen_texture_height;
	int screen_texture_height_old;

	int rec_count;
	int rec_width;
	int rec_height;

	VertexTexCoord_t vertexFormat[4];
	
	QOpenGLShaderProgram *main_shader;
	
	QOpenGLVertexArrayObject *vertex_screen;
	QOpenGLBuffer *buffer_screen_vertex;
	
	VertexTexCoord_t vertexBitmap[4];
	QOpenGLShaderProgram *bitmap_shader;
	QOpenGLBuffer *buffer_bitmap_vertex;
	QOpenGLVertexArrayObject *vertex_bitmap;
	QOpenGLVertexArrayObject *vertex_button[128];
	QOpenGLBuffer *buffer_button_vertex[128];
	QOpenGLShaderProgram *button_shader;

	GLuint uVramTextureID;
	GLuint uButtonTextureID[128];
	GLfloat fButtonX[128];
	GLfloat fButtonY[128];
	GLfloat fButtonWidth[128];
	GLfloat fButtonHeight[128];
	QVector<VertexTexCoord_t> *vertexButtons;

	bool button_updated;
	void updateButtonTexture(void);

	GLfloat fBrightR;
	GLfloat fBrightG;
	GLfloat fBrightB;
	bool set_brightness;
	bool InitVideo;
	GLuint uBitmapTextureID;
	bool bitmap_uploaded;
	virtual void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
					  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4);
	
	virtual void resizeGL_Screen(void);
	virtual void drawGridsHorizonal(void);
	virtual void drawGridsVertical(void);
	void drawGridsMain(GLfloat *tp,
					   int number,
					   GLfloat lineWidth = 0.2f,
					   QVector4D color = QVector4D(0.0f, 0.0f, 0.0f, 1.0f));
	void drawButtons();
	bool button_drawn;
	void drawBitmapTexture(void);
	bool crt_flag;
	bool redraw_required;

	QOpenGLFramebufferObject *offscreen_frame_buffer;
	QOpenGLFramebufferObjectFormat *offscreen_frame_buffer_format;
	QImage offscreen_image;
	GLint texture_max_size;
	bool low_resolution_screen;
public:
	GLDraw_2_0(GLDrawClass *parent, USING_FLAGS *p, EMU *emu = 0);
	~GLDraw_2_0();

	virtual void initGLObjects();
	virtual void initFBO(void);
	virtual void initLocalGLObjects(void);
	virtual void uploadMainTexture(QImage *p, bool chromakey);

	virtual void drawScreenTexture(void);
	void drawGrids(void);
	void uploadBitmapTexture(QImage *p);

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
	virtual void do_set_screen_multiply(float mul);
	
	void initializeGL();
	void paintGL();
	void resizeGL(int width, int height);

	void setImgPtr(QImage *p);
	void setSmoosing(bool);
	void setDrawGLGridVert(bool);
	void setDrawGLGridHoriz(bool);
	void setVirtualVramSize(int ,int);	
	void setEmuPtr(EMU *p);
	void setChangeBrightness(bool);
	void doSetGridsHorizonal(int lines, bool force);
	void doSetGridsVertical(int pixels, bool force);
	void updateBitmap(QImage *);
	void paintGL_OffScreen(int count, int w, int h);

signals:
	int sig_push_image_to_movie(int, int, int, QImage *);
};
#endif // _QT_COMMON_GLUTIL_2_0_H
