/*
 * agar_gldraw2.cpp
 * Using Indexed palette @8Colors.
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */

#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>

#include <SDL/SDL.h>
#ifdef _WINDOWS
#include <GL/gl.h>
#include <GL/glext.h>
#else
#include <GL/glx.h>
#include <GL/glxext.h>
#endif

#undef _USE_OPENCL
#ifdef _USE_OPENCL
# include "agar_glcl.h"
#endif

#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP

#include "agar_gldraw.h"
#include "agar_glutil.h"
#include "agar_main.h"
#include "emu.h"


GLuint uVramTextureID;
GLuint uNullTextureID;
#ifdef _USE_OPENCL
extern class GLCLDraw *cldraw;
extern void InitContextCL(void);
#endif


// Grids
extern GLfloat *GridVertexs200l;
extern GLfloat *GridVertexs400l;

// Brights
float fBrightR;
float fBrightG;
float fBrightB;


void SetBrightRGB_AG_GL2(float r, float g, float b)
{
   fBrightR = r;
   fBrightG = g;
   fBrightB = b;
//   SDLDrawFlag.Drawn = TRUE; // Force draw.
}



/*
 * Event Functins
 */

void AGEventOverlayGL(AG_Event *event)
{
	AG_GLView *glv = (AG_GLView *)AG_SELF();
}


void AGEventScaleGL(AG_Event *event)
{
   AG_GLView *glv = (AG_GLView *)AG_SELF();

   glViewport(glv->wid.rView.x1, glv->wid.rView.y1, glv->wid.rView.w, glv->wid.rView.h);
    //glLoadIdentity();
    //glOrtho(-1.0, 1.0,	1.0, -1.0, -1.0,  1.0);

}


static void drawGrids(void *pg,int w, int h)
{
    AG_GLView *glv = (AG_GLView *)pg;

   
}


static void drawUpdateTexture(Uint32 *p, int w, int h, BOOL crtflag)
{
    if(uVramTextureID != 0){
       Uint32 *pu;
       Uint32 *pq;
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

#ifdef _USE_OPENCL
       if((cldraw != NULL) && (bCLEnabled)) {
 	  cl_int ret = CL_SUCCESS;
	  LockVram();
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
		  glBindTexture(GL_TEXTURE_2D, 0);
		  UnlockVram();
		  return;
		}
	    }
	    if(bCLGLInterop){
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, cldraw->GetPbo());
	        glBindTexture(GL_TEXTURE_2D, uVramTextureID);
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
		glBindTexture(GL_TEXTURE_2D, 0);
	        glFinish();
	    } else { // Not interoperability with GL
		Uint32 *pp;
		pp = cldraw->GetPixelBuffer();
	        glBindTexture(GL_TEXTURE_2D, uVramTextureID);
		if(pp != NULL) glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
					      w, h, GL_RGBA, GL_UNSIGNED_BYTE, pp);
	        glFinish();
		cldraw->ReleasePixelBuffer(pp);
		glBindTexture(GL_TEXTURE_2D, 0);
	        glFinish();
	    }
	    //SDLDrawFlag.Drawn = FALSE;
	    bPaletFlag = FALSE;
	  UnlockVram();
       } else {
#endif
	  //LockVram();
	  flag = TRUE;
	  //flag |= SDLDrawFlag.Drawn;
	  if((p != NULL) && (flag)) {
	     if(crtflag != FALSE) {
		glBindTexture(GL_TEXTURE_2D, uVramTextureID);
		glTexSubImage2D(GL_TEXTURE_2D, 
			  0,
			  0,
			  0,
			  640,
			  h,
			  GL_RGBA,
			  GL_UNSIGNED_BYTE,
			  p);
	       glFinish();
	       glBindTexture(GL_TEXTURE_2D, 0); // 20111023 チラつきなど抑止
	     }
	     //bPaletFlag = FALSE;
	     //SDLDrawFlag.Drawn = FALSE;
	  }
	  //UnlockVram();
#ifdef _USE_OPENCL
       }
#endif       
    }
}

   




/*
 * "Draw"イベントハンドラ
 */

void AGEventDrawGL2(AG_Event *event)
{
   AG_GLView *glv = (AG_GLView *)AG_SELF();
   int w;
   int h;
   int i;
   float width;
   float yf;
   Uint32 *p;
   Uint32 *pp;
   int x;
   int y;
   GLfloat TexCoords[4][2];
   GLfloat Vertexs[4][3];
   GLfloat TexCoords2[4][2];
   GLfloat *gridtid;
   BOOL crtflag = true;
   
   p = emu->screen_buffer(0);
   w = SCREEN_WIDTH;
   h = SCREEN_HEIGHT;
   if((p == NULL) && (bCLEnabled == FALSE)) return;
   TexCoords[0][0] = TexCoords[3][0] = 0.0f; // Xbegin
   TexCoords[0][1] = TexCoords[1][1] = 0.0f; // Ybegin
   
   TexCoords[2][0] = TexCoords[1][0] = (float)w / (float)w; // Xend
   TexCoords[2][1] = TexCoords[3][1] = (float)(h - 1) / (float)h; // Yend
   gridtid = GridVertexs400l;

    Vertexs[0][2] = Vertexs[1][2] = Vertexs[2][2] = Vertexs[3][2] = -0.98f;
    Vertexs[0][0] = Vertexs[3][0] = -1.0f; // Xbegin
    Vertexs[0][1] = Vertexs[1][1] = 1.0f;  // Yend
    Vertexs[2][0] = Vertexs[1][0] = 1.0f; // Xend
    Vertexs[2][1] = Vertexs[3][1] = -1.0f; // Ybegin


    if(uVramTextureID == 0) uVramTextureID = CreateNullTexture(640, 400); //  ドットゴーストを防ぐ
    if(uNullTextureID == 0) uNullTextureID = CreateNullTexture(640, 400); //  ドットゴーストを防ぐ
     /*
     * 20110904 OOPS! Updating-Texture must be in Draw-Event-Handler(--;
     */

    glPushAttrib(GL_TEXTURE_BIT);
    glPushAttrib(GL_TRANSFORM_BIT);
    glPushAttrib(GL_ENABLE_BIT);
#ifdef _USE_OPENCL
    InitContextCL();   
#endif
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
   
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
   
    /*
     * VRAMの表示:テクスチャ貼った四角形
     */
     //if(uVramTextureID != 0) {

       if(crtflag){
	  drawUpdateTexture(p, w, h, crtflag);
	  glEnable(GL_TEXTURE_2D);
	  glBindTexture(GL_TEXTURE_2D, uVramTextureID);
	  glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
     //} else {
//	glDisable(GL_TEXTURE_2D);
//	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
//     }
     } else {
	   glEnable(GL_TEXTURE_2D);
	  glBindTexture(GL_TEXTURE_2D, uNullTextureID);
	   glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
     }	     
       //if(!bSmoosing) {
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	 glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
       //} else {
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      // }
       if(bGL_EXT_VERTEX_ARRAY) {
	 glEnable(GL_TEXTURE_COORD_ARRAY_EXT);
	 glEnable(GL_VERTEX_ARRAY_EXT);
	      
	 glTexCoordPointerEXT(2, GL_FLOAT, 0, 4, TexCoords);
	 glVertexPointerEXT(3, GL_FLOAT, 0, 4, Vertexs);
	 glDrawArraysEXT(GL_POLYGON, 0, 4);
	 
	 glDisable(GL_VERTEX_ARRAY_EXT);
	 glDisable(GL_TEXTURE_COORD_ARRAY_EXT);
       } else {
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
       }
    // }
   
     // 20120502 輝度調整
    glBindTexture(GL_TEXTURE_2D, 0); // 20111023
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);

    if(bCLEnabled == FALSE){
       glEnable(GL_BLEND);
   
       glColor3f(fBrightR , fBrightG, fBrightB);
       glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    
       //    glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
       if(bGL_EXT_VERTEX_ARRAY) {
	  glEnable(GL_VERTEX_ARRAY_EXT);
	  glVertexPointerEXT(3, GL_FLOAT, 0, 4, Vertexs);
	  glDrawArraysEXT(GL_POLYGON, 0, 4);
	  glDisable(GL_VERTEX_ARRAY_EXT);
       } else {
	  glBegin(GL_POLYGON);
	  glVertex3f(Vertexs[0][0], Vertexs[0][1], Vertexs[0][2]);
	  glVertex3f(Vertexs[1][0], Vertexs[1][1], Vertexs[1][2]);
	  glVertex3f(Vertexs[2][0], Vertexs[2][1], Vertexs[2][2]);
	  glVertex3f(Vertexs[3][0], Vertexs[3][1], Vertexs[3][2]);
	  glEnd();
       }
       
       glBlendFunc(GL_ONE, GL_ZERO);
   
       glDisable(GL_BLEND);
    }
       glDisable(GL_TEXTURE_2D);
       glDisable(GL_DEPTH_TEST);
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
}

