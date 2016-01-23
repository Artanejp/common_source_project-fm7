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
#if defined(MAX_BUTTONS) || defined(ONE_BOARD_MICRO_COMPUTER)
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QRect>
#endif
#include <QGLWidget>
#include <QImage>
#include <QOpenGLFunctions_2_0>
#include <QTimer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLContext>

#include <QMatrix4x2>
#include <QMatrix4x4>

#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

#include "common.h"
#include "osd.h"
#include "emu.h"

typedef struct  {
		GLfloat x, y, z;
		GLfloat s, t;
} VertexTexCoord_t;
typedef struct {
		GLfloat x, y;
} VertexLines_t ;

class EMU;
class QEvent;
class GLDrawClass;

class GLDraw_2_0 : public QObject
{
	Q_OBJECT
protected:
	GLDrawClass *p_wid;
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

	QOpenGLFunctions_2_0 *extfunc;
	VertexTexCoord_t vertexFormat[4];
	
	QOpenGLShaderProgram *main_shader;
	
	QOpenGLVertexArrayObject *vertex_screen;
	QOpenGLBuffer *buffer_screen_vertex;
	
# if defined(ONE_BOARD_MICRO_COMPUTER)
	VertexTexCoord_t vertexBitmap[4];
	QOpenGLShaderProgram *bitmap_shader;
	QOpenGLBuffer *buffer_bitmap_vertex;
	QOpenGLVertexArrayObject *vertex_bitmap;
# endif
# if defined(MAX_BUTTONS)
	QOpenGLVertexArrayObject *vertex_button[MAX_BUTTONS];
	QOpenGLBuffer *buffer_button_vertex[MAX_BUTTONS];
	QOpenGLShaderProgram *button_shader[MAX_BUTTONS];
# endif	

	GLuint uVramTextureID;
	
#if defined(MAX_BUTTONS)
	GLuint uButtonTextureID[MAX_BUTTONS];
	GLfloat fButtonX[MAX_BUTTONS];
	GLfloat fButtonY[MAX_BUTTONS];
	GLfloat fButtonWidth[MAX_BUTTONS];
	GLfloat fButtonHeight[MAX_BUTTONS];
	QVector<VertexTexCoord_t> *vertexButtons;

	bool button_updated;
	void updateButtonTexture(void);
#endif
	GLfloat fBrightR;
	GLfloat fBrightG;
	GLfloat fBrightB;
	bool set_brightness;
	bool InitVideo;
#ifdef ONE_BOARD_MICRO_COMPUTER
	GLuint uBitmapTextureID;
	bool bitmap_uploaded;
#endif
	virtual void setNormalVAO(QOpenGLShaderProgram *prg, QOpenGLVertexArrayObject *vp,
					  QOpenGLBuffer *bp, VertexTexCoord_t *tp, int size = 4);
	
	virtual void drawGridsHorizonal(void);
	virtual void drawGridsVertical(void);
	void drawGridsMain(GLfloat *tp,
					   int number,
					   GLfloat lineWidth = 0.2f,
					   QVector4D color = QVector4D(0.0f, 0.0f, 0.0f, 1.0f));
#if defined(MAX_BUTTONS)
	void drawButtons();
	bool button_drawn;
#endif
#ifdef ONE_BOARD_MICRO_COMPUTER
	void drawBitmapTexture(void);
#endif
	bool crt_flag;
	bool redraw_required;
	
public:
	GLDraw_2_0(GLDrawClass *parent, EMU *emu = 0);
	~GLDraw_2_0();

	virtual void initGLObjects();
	void initFBO(void);
	virtual void initLocalGLObjects(void);
	virtual void uploadMainTexture(QImage *p, bool chromakey);

	virtual void drawScreenTexture(void);
	void drawGrids(void);
#ifdef ONE_BOARD_MICRO_COMPUTER
	void uploadBitmapTexture(QImage *p);
#endif	

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
	void do_set_screen_multiply(float mul);
	
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
#ifdef ONE_BOARD_MICRO_COMPUTER
	void updateBitmap(QImage *);
#endif   
};
#endif // _QT_COMMON_GLUTIL_2_0_H
