/*
 * qt_gldraw.cpp
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */


#include "emu.h"

#include <QtGui>
#include <QOpenGLWidget>
//#include <SDL/SDL.h>
#ifdef _WINDOWS
#include <GL/gl.h>
#include <GL/glext.h>
#else
#include <GL/glx.h>
#include <GL/glxext.h>
#endif
#include <GL/glu.h>

#undef _USE_OPENCL
#ifdef _USE_OPENCL
//# include "agar_glcl.h"
#endif


#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP
#include "qt_gldraw.h"
#include "agar_logger.h"

void GLDrawClass::SetBrightRGB(float r, float g, float b)
{
	fBrightR = r;
	fBrightG = g;
	fBrightB = b;
	//   SDLDrawFlag.Drawn = TRUE; // Force draw.
}


#ifdef _USE_OPENCL
extern class GLCLDraw *cldraw;
extern void InitContextCL(void);
#endif

// Grids

// Brights


void GLDrawClass::drawGrids(void *pg,int w, int h)
{

   
}


void GLDrawClass::drawUpdateTexture(QImage *p)
{
	if((p != NULL)) {
		if(uVramTextureID->isCreated()) {
	  		uVramTextureID->destroy();
			uVramTextureID->create();
		}
		uVramTextureID->setData(*p);
	}
//#ifdef _USE_OPENCL
}

#if defined(USE_BITMAP)
void GLDrawClass::uploadBitmapTexture(QImage *p)
{
	int i;
	if(p == NULL) return;
	if(!bitmap_uploaded) {
		if(uBitmapTextureID->isCreated()) {
	  		uBitmapTextureID->destroy();
			uBitmapTextureID->create();
		}
		uBitmapTextureID->setData(*p);
		bitmap_uploaded = true;
		crt_flag = true;
	}
}

void GLDrawClass::updateBitmap(QImage *p)
{
	bitmap_uploaded = false;
	uploadBitmapTexture(p);
}
#endif

void GLDrawClass::resizeGL(int width, int height)
{
	int side = qMin(width, height);
	double ww, hh;
	double ratio;
	int w, h;
	if(p_emu == NULL) return;
	ww = (double)width;
	hh = (double)height;
	switch(config.stretch_type) {
	case 0: // Dot by Dot
		//ratio = (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT;
#ifdef USE_SCREEN_ROTATE
		if(config.rotate_type) {
			ratio =  (double)p_emu->get_screen_height_aspect() / (double)p_emu->get_screen_width_aspect();
			h = (int)(ww / ratio);
			w = (int)(hh * ratio);
		} else
#endif	   
		{
			ratio =  (double)p_emu->get_screen_width_aspect() / (double)p_emu->get_screen_height_aspect();
			h = (int)(ww / ratio);
			w = (int)(hh * ratio);
		}
		break;
	case 1: // Keep Aspect
#ifdef USE_SCREEN_ROTATE
		if(config.rotate_type) {
			ratio = (double)WINDOW_HEIGHT_ASPECT / (double)WINDOW_WIDTH_ASPECT;
			h = (int)(ww / ratio);
			w = (int)(hh * ratio);
		} else
#endif	   
		{
			ratio = (double)WINDOW_WIDTH_ASPECT / (double)WINDOW_HEIGHT_ASPECT;
			h = (int)(ww / ratio);
			w = (int)(hh * ratio);
		}
		break;
	case 2: // Fill
	default:
		h = height;
		w = width;
		ratio = (double)width / (double)height;
		break;
	}
	if(h > height) {
		h = height;
		w = (int)((double)h * ratio);
	}
	if(w > width) {
		w = width;
		h = (int)((double)w / ratio);
	}
	glViewport((width - w) / 2, (height - h) / 2, w, h);
	draw_width = w;
	draw_height = h;
	crt_flag = true;
   
	AGAR_DebugLog(AGAR_LOG_DEBUG, "ResizeGL: %dx%d", width , height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#ifdef QT_OPENGL_ES_1
	glOrthof(-1.0, 1.0, -1.0, +1.0, -1.0, 1.0);
#else
	glOrtho(-1.0, 1.0, -1.0, +1.0, -1.0, 1.0);
#endif
}

/*
 * "Paint" Event handler
 */

void GLDrawClass::paintGL(void)
{
	int i;
	float yf;
	QImage *p;
	GLfloat TexCoords[4][2];
	GLfloat Vertexs[4][3];
	GLfloat TexCoords2[4][2];
	GLfloat *gridtid;
	GLfloat w, h;
	if(!crt_flag) return;
	if(p_emu != NULL) {
		if(imgptr == NULL) return;
		drawUpdateTexture(imgptr);
		crt_flag = false;
	}
	w = ((GLfloat) draw_width / (GLfloat)(this->width())); 
	h = ((GLfloat) draw_height / (GLfloat)(this->height()));
	//w = h = 1.0f;
	TexCoords[0][0] = TexCoords[3][0] = 0.0f; // Xbegin
	TexCoords[0][1] = TexCoords[1][1] = 0.0f; // Ybegin
   
	TexCoords[2][0] = TexCoords[1][0] = 1.0f; // Xend
	TexCoords[2][1] = TexCoords[3][1] = 1.0f; // Yend

	Vertexs[0][2] = Vertexs[1][2] = Vertexs[2][2] = Vertexs[3][2] = -0.0f;
	Vertexs[0][0] = Vertexs[3][0] = -w; // Xbegin
	Vertexs[0][1] = Vertexs[1][1] = h;  // Yend
	Vertexs[2][0] = Vertexs[1][0] = w; // Xend
	Vertexs[2][1] = Vertexs[3][1] = -h; // Ybegin
	/*
	 * 20110904 OOPS! Updating-Texture must be in Draw-Event-Handler(--;
	 */

	glPushAttrib(GL_TEXTURE_BIT);
	glPushAttrib(GL_TRANSFORM_BIT);
	glPushAttrib(GL_ENABLE_BIT);
#ifdef _USE_OPENCL
	//    InitContextCL();   
#endif
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	
	/*
	 * VRAMの表示:テクスチャ貼った四角形
	 */
	glEnable(GL_TEXTURE_2D);
	uVramTextureID->bind();
	//if(!bSmoosing) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		// drawTexture(QPointF(0,0),uVramTextureID,GL_TEXTURE_2D);
	//} else {
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//}
#ifdef USE_SCREEN_ROTATE   
	glPushMatrix();
	//if(config.rotate_type) glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
#endif   
	glBegin(GL_POLYGON);
	glTexCoord2f(TexCoords[0][0], TexCoords[0][1]);
	glVertex3f(Vertexs[0][0], Vertexs[0][1], Vertexs[0][2]);
	
	glTexCoord2f(TexCoords[1][0], TexCoords[1][1]);
	glVertex3f(Vertexs[1][0], Vertexs[1][1], Vertexs[1][2]);
	 
	glTexCoord2f(TexCoords[2][0], TexCoords[2][1]);
	glVertex3f(Vertexs[2][0], Vertexs[2][1], Vertexs[2][2]);
	      
	glTexCoord2f(TexCoords[3][0], TexCoords[3][1]);
	glVertex3f(Vertexs[3][0], Vertexs[3][1], Vertexs[3][2]);
	glEnd();
	// }
	// 20120502 輝度調整
	uVramTextureID->release();
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
//    if(bCLEnabled == FALSE){
	glEnable(GL_BLEND);
   
	glColor3f(fBrightR , fBrightG, fBrightB);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    
	//    glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
	//       if(bGL_EXT_VERTEX_ARRAY) {
	//	  glEnable(GL_VERTEX_ARRAY_EXT);
	//	  glVertexPointerEXT(3, GL_FLOAT, 0, 4, Vertexs);
	//	  glDrawArraysEXT(GL_POLYGON, 0, 4);
	//	  glDisable(GL_VERTEX_ARRAY_EXT);
	//       } else {
	glBegin(GL_POLYGON);
	glVertex3f(Vertexs[0][0], Vertexs[0][1], Vertexs[0][2]);
	glVertex3f(Vertexs[1][0], Vertexs[1][1], Vertexs[1][2]);
	glVertex3f(Vertexs[2][0], Vertexs[2][1], Vertexs[2][2]);
	glVertex3f(Vertexs[3][0], Vertexs[3][1], Vertexs[3][2]);
	glEnd();
	//       }
       
	glBlendFunc(GL_ONE, GL_ZERO);
   
	glDisable(GL_BLEND);
//    }
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
# if 0
        if(glv->wid.rView.h >= h) {
	  glLineWidth((float)(glv->wid.rView.h) / (float)(h * 2));
	  glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	  if(bGL_EXT_VERTEX_ARRAY) {
	     glEnable(GL_VERTEX_ARRAY_EXT);
	     glVertexPointerEXT(3, GL_FLOAT, 0, h + 1, gridtid);
	     glDrawArraysEXT(GL_LINE, 0, h + 1);
	     glDisable(GL_VERTEX_ARRAY_EXT);
	  } else {
	     glBegin(GL_LINES);
	     for(y = 0; y < h; y++) {
		yf = -1.0f + (float) (y + 1) * 2.0f / (float)h;
		glVertex3f(-1.0f, yf, 0.96f);  
		glVertex3f(+1.0f, yf, 0.96f);  
	     }
	     glEnd();
	  }
       
	}
#endif   
   //}
#ifdef USE_SCREEN_ROTATE   
	glPopMatrix();
#endif   
	glDisable(GL_DEPTH_TEST);
#ifdef USE_OPENGL
	//DrawOSDGL(glv);
#endif
	glPopAttrib();
	glPopAttrib();
	glPopAttrib();
	glFlush();
	//    swapBuffers();
}

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLDrawClass::GLDrawClass(QWidget *parent)
  : QOpenGLWidget(parent, Qt::Widget)
{
	uVramTextureID = new QOpenGLTexture(QOpenGLTexture::Target2D);
	imgptr = NULL;
	p_emu = NULL;
#ifdef USE_BITMAP
	uBitmapTextureID = new QOpenGLTexture(QOpenGLTexture::Target2D);
#endif	
        fBrightR = 1.0; // 輝度の初期化
        fBrightG = 1.0;
        fBrightB = 1.0;
	crt_flag = false;
#ifdef _USE_OPENCL
        bInitCL = false;
        nCLGlobalWorkThreads = 10;
        bCLSparse = false; // true=Multi threaded CL,false = Single Thread.
	nCLPlatformNum = 0;
	nCLDeviceNum = 0;
	bCLInteropGL = false;
        keyin_lasttime = SDL_GetTicks();
    //bCLDirectMapping = false;
#endif
#ifdef USE_BITMAP
	uBitMapTextureID = 0;
	bitmap_uploaded = false;
#endif   
        this->setFocusPolicy(Qt::StrongFocus);
}

GLDrawClass::~GLDrawClass()
{
	delete uVramTextureID;
#ifdef USE_BITMAP
	delete uBitmapTextureID;
#endif
}

void GLDrawClass::setEmuPtr(EMU *p)
{
	p_emu = p;
}

QSize GLDrawClass::minimumSizeHint() const
{
	return QSize(50, 50);
}

QSize GLDrawClass::sizeHint() const
{
	return QSize(400, 400);
}

QSize GLDrawClass::getCanvasSize(void)
{
	return QSize(this->width(), this->height());
}

QSize GLDrawClass::getDrawSize(void)
{
	return QSize(draw_width, draw_height);
}

