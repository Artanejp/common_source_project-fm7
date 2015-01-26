/*
 * qt_gldraw.cpp
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */


#include "emu.h"

#include <QtGui>
#include <QtOpenGL/QGLWidget>
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
//#include "agar_main.h"

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


void GLDrawClass::drawUpdateTexture(QImage *p, int w, int h, bool crtflag)
{
       uint32_t *pu;
       uint32_t *pq;
       int xx;
       int yy;
       int ww;
       int hh;
       int ofset;
       BOOL flag;
       int i;
       //       glPushAttrib(GL_TEXTURE_BIT);
       ww = w >> 3;
       hh = h >> 3;
       crtflag = true;
//#ifdef _USE_OPENCL
#if 0
       if((cldraw != NULL) && (bCLEnabled)) {
 	  cl_int ret = CL_SUCCESS;
//	  LockVram();
	  flag = FALSE;
	  for(i = 0; i < h; i++) {
	    if(bDrawLine[i]) {
	      flag = TRUE;
	    }
	  }
	  //if(SDLDrawFlag.Drawn) flag = TRUE;
	  if(flag) {
		ret = cldraw->GetVram(bModeOld);
	        for(i = 0; i < h; i++)	bDrawLine[i] = FALSE;

		if(ret != CL_SUCCESS) {
		  //SDLDrawFlag.Drawn = FALSE;
		  bPaletFlag = FALSE;
		  this->bindTexture(GL_TEXTURE_2D, 0);
		  //UnlockVram();
		  return;
		}
	    }
	    if(bCLGLInterop){
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, cldraw->GetPbo());
	        this->bindTexture(GL_TEXTURE_2D, uVramTextureID);
		// Copy pbo to texture 
		glTexSubImage2D(GL_TEXTURE_2D, 
				0,
				0,
				0,
				w,
				h,
				GL_RGBA,
				GL_UNSIGNED_BYTE,
				NULL);
	        glFinish();
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		this->indTexture(GL_TEXTURE_2D, 0);
	        glFinish();
	    } else { // Not interoperability with GL
		uint32_t *pp;
		pp = cldraw->GetPixelBuffer();
	        this->indTexture(GL_TEXTURE_2D, uVramTextureID);
		if(pp != NULL) glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
					      w, h, GL_RGBA, GL_UNSIGNED_BYTE, pp);
	        glFinish();
		cldraw->ReleasePixelBuffer(pp);
		glBindTexture(GL_TEXTURE_2D, 0);
	        glFinish();
	    }
	    //SDLDrawFlag.Drawn = FALSE;
	    bPaletFlag = FALSE;
	  //UnlockVram();
       } else {
#endif
	  //LockVram();
	  flag = TRUE;
	  //flag |= SDLDrawFlag.Drawn;
	  if((p != NULL)) {
	     if(uVramTextureID != 0) deleteTexture(uVramTextureID);
             uVramTextureID = QGLWidget::bindTexture(*p, GL_TEXTURE_2D, GL_RGBA);
	  }
//#ifdef _USE_OPENCL
#if 0
       }
#endif       

}

void GLDrawClass::resizeGL(int width, int height)
{
   int side = qMin(width, height);
   double ww, hh;
   double ratio;
   int w, h;
   if(emu == NULL) return;
   ww = (double)width;
   hh = (double)height;
#if 1
   switch(config.stretch_type) {
   case 0: // Dot by Dot
     ratio = (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT;
     h = (int)(ww / ratio);
     w = (int)(hh * ratio);
     break;
   case 1: // Keep Aspect
     ratio =  (double)emu->get_screen_width_aspect() / (double)emu->get_screen_height_aspect();
     h = (int)(ww / ratio);
     w = (int)(hh * ratio);
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
#else
   glViewport(0, 0, width, height);
#endif
   printf("ResizeGL: %dx%d\n", width , height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
#ifdef QT_OPENGL_ES_1
   glOrthof(-1.0, 1.0, +1.0, -1.0, -1.0, 1.0);
#else
   glOrtho(-1.0, 1.0, +1.0, -1.0, -1.0, 1.0);
#endif
//   glMatrixMode(GL_MODELVIEW);//    glLoadIdentity();
}

/*
 * "Paint"イベントハンドラ
 */

void GLDrawClass::paintGL(void)
{
   int w;
   int h;
   int i;
   float width;
   float yf;
   QImage *p;
   uint32_t *pp;
   int x;
   int y;
   GLfloat TexCoords[4][2];
   GLfloat Vertexs[4][3];
   GLfloat TexCoords2[4][2];
   GLfloat *gridtid;
   bool crtflag = true;
   
   if(emu == NULL) return;
   w = SCREEN_WIDTH;
   h = SCREEN_HEIGHT;
   p = emu->getPseudoVramClass();
   if(p == NULL) return;
   TexCoords[0][0] = TexCoords[3][0] = 0.0f; // Xbegin
   TexCoords[0][1] = TexCoords[1][1] = 0.0f; // Ybegin
   
   TexCoords[2][0] = TexCoords[1][0] = 1.0f; // Xend
   TexCoords[2][1] = TexCoords[3][1] = 1.0f; // Yend
//   gridtid = GridVertexs400l;

    Vertexs[0][2] = Vertexs[1][2] = Vertexs[2][2] = Vertexs[3][2] = -0.0f;
    Vertexs[0][0] = Vertexs[3][0] = -1.0f; // Xbegin
    Vertexs[0][1] = Vertexs[1][1] = 1.0f;  // Yend
    Vertexs[2][0] = Vertexs[1][0] = 1.0f; // Xend
    Vertexs[2][1] = Vertexs[3][1] = -1.0f; // Ybegin


//    if(uVramTextureID == 0) uVramTextureID = CreateNullTexture(640, 400); //  ドットゴーストを防ぐ
    if(uNullTextureID == 0) uNullTextureID = CreateNullTexture(640, 400); //  ドットゴーストを防ぐ
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
     //if(uVramTextureID != 0) {

//       if(crtflag){
	  glEnable(GL_TEXTURE_2D);
	  drawUpdateTexture(p, w, h, crtflag);
//	  glEnable(GL_TEXTURE_2D);
//	  glBindTexture(GL_TEXTURE_2D, uVramTextureID);
	  //glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//     } else {
//	   glEnable(GL_TEXTURE_2D);
//	  glBindTexture(GL_TEXTURE_2D, uNullTextureID);
//	   glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//     }	     
       //if(!bSmoosing) {
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//         drawTexture(QPointF(0,0),uVramTextureID,GL_TEXTURE_2D);
   //} else {
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      // }
       //if(bGL_EXT_VERTEX_ARRAY) {
//	 glEnable(GL_TEXTURE_COORD_ARRAY_EXT);
//	 glEnable(GL_VERTEX_ARRAY_EXT);
	      
//	 glTexCoordPointerEXT(2, GL_FLOAT, 0, 4, TexCoords);
//	 glVertexPointerEXT(3, GL_FLOAT, 0, 4, Vertexs);
//	 glDrawArraysEXT(GL_POLYGON, 0, 4);
	 
//	 glDisable(GL_VERTEX_ARRAY_EXT);
//	 glDisable(GL_TEXTURE_COORD_ARRAY_EXT);
//       } else {
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
    glBindTexture(GL_TEXTURE_2D, 0); // 20111023
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
   glDisable(GL_BLEND);
   glDisable(GL_TEXTURE_2D);
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
     : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
 {
	uVramTextureID = 0;
	uNullTextureID = 0;
        fBrightR = 1.0; // 輝度の初期化
        fBrightG = 1.0;
        fBrightB = 1.0;
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
        this->setFocusPolicy(Qt::StrongFocus);
 }

GLDrawClass::~GLDrawClass()
{
}

QSize GLDrawClass::minimumSizeHint() const
{
     return QSize(50, 50);
}

QSize GLDrawClass::sizeHint() const
 {
     return QSize(400, 400);
 }

