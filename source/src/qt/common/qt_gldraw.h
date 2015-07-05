/*
 * qt_gldraw.h
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */
#ifndef _CSP_QT_GLDRAW_H
#define _CSP_QT_GLDRAW_H

#include "emu.h"

#include <QOpenGLWidget>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QOpenGLContext>
#include <QOpenGLFunctions_2_0>

#include <GL/gl.h>
#include <QTimer>

class EMU;
class GLDrawClass: public QOpenGLWidget 
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
	QOpenGLFunctions_2_0 *extfunc;   

 protected:
	void keyReleaseEvent(QKeyEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void initializeGL();
	void paintGL();
	
	QOpenGLTexture *uVramTextureID;
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
	QOpenGLTexture *uBitMapTextureID;
	bool bitmap_uploaded;
	void uploadBitmapTexture(QImage *p);
#endif
#ifdef _USE_OPENCL
	//     extern class GLCLDraw *cldraw;
#endif
	bool QueryGLExtensions(const char *str);
	void InitGLExtensionVars(void);
	void InitContextCL(void);
	void drawUpdateTexture(QImage *p);
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
	void update_screen(void);
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
};

#endif // End.
