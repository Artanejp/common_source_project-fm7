/*
 * qt_gldraw.h
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */
#ifndef _CSP_QT_GLDRAW_H
#define _CSP_QT_GLDRAW_H

#include "emu.h"

#if (QT_MAJOR_VERSION >= 5)
# if (QT_MINOR_VERSION >= 4) && defined(_USE_QT_5_4)
#  include <QOpenGLWidget>
#  include <QOpenGLTexture>
#  include <QOpenGLFunctions>
#  include <QOpenGLContext>
//#  include <QOpenGLFunctions_2_0>
#  define _USE_GLAPI_QT5_4
# elif (QT_MINOR_VERSION >= 1)
#  include <QGLWidget>
#  include <QOpenGLFunctions>
//#  include <QOpenGLFunctions_2_0>
#  define _USE_GLAPI_QT5_1
# endif
#elif (QT_MAJOR_VERSION == 4)
# if (QT_MINOR_VERSION >= 8)
#  include <QGLWidget>
#  include <QGLFunctions>
#  define _USE_GLAPI_QT4_8
# endif

#else

// TO DO IMPLEMENT.

#endif

#include <GL/gl.h>
#include <QTimer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_0>

#include <QMatrix4x2>
#include <QMatrix4x4>

#include <QVector>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

class EMU;

struct NativeScanCode {
	uint32_t vk;
	uint32_t scan;
};
struct NativeVirtualKeyCode {
	uint32_t vk;
	uint32_t key;
};

typedef struct VertexTexCoord_t {
		GLfloat x, y, z;
		GLfloat s, t;
};
typedef struct VertexLines_t {
		GLfloat x, y;
};

#if defined(_USE_GLAPI_QT5_4)
class GLDrawClass: public QOpenGLWidget 
#else
class GLDrawClass: public QGLWidget 
#endif
{
	Q_OBJECT
 private:
	int draw_width;
	int draw_height;
	EMU *p_emu;
	QImage *imgptr;
	bool enable_mouse;
	GLfloat screen_width, screen_height;
	bool smoosing;
	bool gl_grid_horiz;
	bool gl_grid_vert;
	int  vert_lines;
	int  horiz_pixels;
	GLfloat *glVertGrids;
	GLfloat *glHorizGrids;
	
	bool req_draw_grids_horiz;
	bool req_draw_grids_vert;

	bool bGL_ARB_IMAGING; // イメージ操作可能か？
	bool bGL_ARB_COPY_BUFFER;  // バッファ内コピー（高速化！）サポート
	bool bGL_EXT_INDEX_TEXTURE; // パレットモードに係わる
	bool bGL_EXT_COPY_TEXTURE; // テクスチャ間のコピー
	bool bGL_SGI_COLOR_TABLE; // パレットモード(SGI拡張)
	bool bGL_SGIS_PIXEL_TEXTURE; // テクスチャアップデート用
	bool bGL_EXT_PACKED_PIXEL; // PackedPixelを使ってアップデートを高速化？
	bool bGL_EXT_VERTEX_ARRAY; // 頂点を配列化して描画を高速化
	bool bGL_EXT_PALETTED_TEXTURE; // パレットモード（更に別拡張)
	bool bGL_PIXEL_UNPACK_BUFFER_BINDING; // ピクセルバッファがあるか？
#if defined(_USE_GLAPI_QT5_4) || defined(_USE_GLAPI_QT5_1)  
	QOpenGLFunctions *extfunc;
#elif defined(_USE_GLAPI_QT4_8)
   	QGLFunctions *extfunc;
#endif   
	VertexTexCoord_t vertexFormat[4];
	QOpenGLShaderProgram *main_shader;
	QOpenGLShaderProgram *grids_shader;
	QOpenGLVertexArrayObject *vertex_grid_horizonal;
	QOpenGLVertexArrayObject *vertex_grid_vertical;
	QOpenGLVertexArrayObject *vertex_screen;
	QOpenGLBuffer *buffer_screen_vertex;
	QOpenGLBuffer *buffer_grid_vertical;
	QOpenGLBuffer *buffer_grid_horizonal;
# if defined(USE_BITMAP)
	VertexTexCoord_t vertexBitmap[4];
	QOpenGLShaderProgram *bitmap_shader;
	QOpenGLBuffer *buffer_bitmap_vertex;
	QOpenGLBuffer *buffer_screen_vertex;
# endif
# if defined(USE_BUTTON)
	QOpenGLVertexArrayObject *vertex_button[MAX_BUTTONS];
	QOpenGLBuffer *buffer_button_vertex[MAX_BUTTONS];
	QOpenGLShaderProgram *button_shader;
# endif	

 protected:
	struct NativeScanCode NativeScanCode[256];
	struct NativeVirtualKeyCode NativeVirtualKeyCode[256];

	void keyReleaseEvent(QKeyEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void initializeGL();
	void paintGL();
#if defined(_USE_GLAPI_QT5_4)   
	QOpenGLTexture *uVramTextureID;
#else
	GLuint uVramTextureID;
#endif
#if defined(USE_BUTTON)
#if defined(_USE_GLAPI_QT5_4)   
	QOpenGLTexture *uButtonTextureID[MAX_BUTTONS];
#else
	GLuint uButtonTextureID[MAX_BUTTONS];
#endif
	GLfloat fButtonX[MAX_BUTTONS];
	GLfloat fButtonY[MAX_BUTTONS];
	GLfloat fButtonWidth[MAX_BUTTONS];
	GLfloat fButtonHeight[MAX_BUTTONS];
	bool button_updated;
	void updateButtonTexture(void);
#endif
	GLfloat fBrightR;
	GLfloat fBrightG;
	GLfloat fBrightB;
        bool set_brightness;

	// Will move to OpenCL
	bool bInitCL;
	bool bCLEnabled;
	bool bCLGLInterop;
	int nCLGlobalWorkThreads;
	bool bCLSparse; // TRUE=Multi threaded CL,FALSE = Single Thread.
	int nCLPlatformNum;
	int nCLDeviceNum;
	bool bCLInteropGL;

	bool InitVideo;
	void drawGrids(void);

	uint32_t get106Scancode2VK(uint32_t data);
	uint32_t getNativeKey2VK(uint32_t data);
#ifdef USE_BITMAP
# if defined(_USE_GLAPI_QT5_4)   
	QOpenGLTexture *uBitmapTextureID;
# else
	GLuint uBitmapTextureID;
# endif
	
	bool bitmap_uploaded;
	void uploadBitmapTexture(QImage *p);
#endif
#ifdef _USE_OPENCL
	//     extern class GLCLDraw *cldraw;
#endif
	bool QueryGLExtensions(const char *str);
	void InitGLExtensionVars(void);
	void InitContextCL(void);
	void adjustBrightness();
	
	void drawUpdateTexture(QImage *p);
	void drawGridsHorizonal(void);
	void drawGridsVertical(void);
	void drawScreenTexture(void);
	
#if defined(USE_BUTTON)
	void drawButtons();
	bool button_drawn;
#endif	
# ifdef USE_BITMAP
	void drawBitmapTexture(void);
#endif
	
public:
	GLDrawClass(QWidget *parent = 0);
	~GLDrawClass();

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	QSize getCanvasSize();
	QSize getDrawSize();
	
	bool crt_flag;
	quint32 getModState(void) { return modifier;}
	quint32 modifier;
	void SetBrightRGB(float r, float g, float b);
	void InitFBO(void);
	
public slots:
	void initKeyCode(void);
	void releaseKeyCode(void);
	
	void update_screen(QImage *);
	void resizeGL(int width, int height);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void setEnableMouse(bool flag);
	void setSmoosing(bool);
	void setDrawGLGridVert(bool);
	void setDrawGLGridHoriz(bool);
	void setVirtualVramSize(int ,int);	
	void setChangeBrightness(bool);
	void setBrightness(GLfloat r, GLfloat g, GLfloat b);

#ifdef USE_BITMAP
	void updateBitmap(QImage *);
#endif   
	void setEmuPtr(EMU *p);
signals:
	void update_screenChanged(int tick);
	void do_notify_move_mouse(int x, int y);
	void sig_toggle_mouse(void);
	void do_notify_button_pressed(Qt::MouseButton button);
	void do_notify_button_released(Qt::MouseButton button);
	void sig_check_grab_mouse(bool);
	void sig_resize_uibar(int, int);
};

#endif // End.
