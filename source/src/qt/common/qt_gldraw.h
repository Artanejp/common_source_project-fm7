
#ifndef _CSP_QT_DIALOG_H
#define _CSP_QT_DIALOG_H

//#include <Qt>
#include <QGLWidget>
#include <GL/gl.h>
#include <QTimer>

class GLDrawClass: public QGLWidget 
{
   Q_OBJECT
  
public:
     GLDrawClass(QWidget *parent = 0);
     ~GLDrawClass();

     QSize minimumSizeHint() const;
     QSize sizeHint() const;
     bool crt_flag;
     GLuint CreateNullTexture(int w, int h);
     GLuint CreateNullTextureCL(int w, int h);
#if 0 // TEST
     GLDrawClass( QWidget *parent = 0 , const char *name = 0 ) {
//        InitGLExtensionVars();
#ifdef _USE_OPENCL
	// Init CL
#endif
	modifier = 0;
     }
#else // TEST
     //GLDrawClass(QWidget *parent)
     //: QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
     //~GLWidget();
#endif // Test
     quint32 getModState(void) { return modifier;}
     quint32 modifier;
     void SetBrightRGB(float r, float g, float b);
     void drawUpdateTexture(QImage *p, int w, int h, bool crtflag);
     //void DrawHandler(void);
     void InitFBO(void);
     void InitGridVertexs(void);
     void DiscardTextures(int n, GLuint *id);
     void DiscardTexture(GLuint tid);
	
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
   
// FBO API
     PFNGLVERTEXPOINTEREXTPROC glVertexPointerEXT;
     PFNGLDRAWARRAYSEXTPROC glDrawArraysEXT;
     PFNGLTEXCOORDPOINTEREXTPROC glTexCoordPointerEXT;

     PFNGLBINDBUFFERPROC glBindBuffer;
     PFNGLBUFFERDATAPROC glBufferData;
     PFNGLGENBUFFERSPROC glGenBuffers;
     PFNGLDELETEBUFFERSPROC glDeleteBuffers;

 public slots:
     void update_screen(int tick);
     void resizeGL(int width, int height);
 signals:
     void update_screenChanged(int tick);
 protected:
     uint32_t keyin_lasttime;
     void keyReleaseEvent(QKeyEvent *event);
     void keyPressEvent(QKeyEvent *event);
     void initializeGL();
     void paintGL();
     void mousePressEvent(QMouseEvent *event);
     void mouseMoveEvent(QMouseEvent *event);
     GLuint uVramTextureID;
     GLuint uNullTextureID;
     float GridVertexs200l[202 * 6];
     float GridVertexs400l[402 * 6];
     float fBrightR;
     float fBrightG;
     float fBrightB;
     QTimer *timer;
     // Will move to OpenCL
     bool bInitCL;
     bool bCLEnabled;
     bool bCLGLInterop;
     int nCLGlobalWorkThreads;
     bool bCLSparse; // TRUE=Multi threaded CL,FALSE = Single Thread.
     int nCLPlatformNum;
     int nCLDeviceNum;
     bool bCLInteropGL;
     //
     bool InitVideo;
     void drawGrids(void *pg,int w, int h);
     //     void initializeGL(void);
     //void paintGL(void);
     //void resizeGL(int width, int height);
     uint32_t get106Scancode2VK(uint32_t data);

#ifdef _USE_OPENCL
//     extern class GLCLDraw *cldraw;
#endif
     bool QueryGLExtensions(const char *str);
     void InitGLExtensionVars(void);
     void InitGridVertexsSub(GLfloat *p, int h);
     void InitContextCL(void);
};

#endif // End.
