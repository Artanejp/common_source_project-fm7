/*
 * qt_gldraw.cpp
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */


#include "emu.h"


#include <QtGui>
#if defined(USE_BUTTON)
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QRect>
#endif
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
}


#ifdef _USE_OPENCL
extern class GLCLDraw *cldraw;
extern void InitContextCL(void);
#endif

void GLDrawClass::drawGrids(void)
{
	if(gl_grid_horiz && (vert_lines > 0) && (glHorizGrids != NULL) && req_draw_grids_vert) {
	  //req_draw_grids_vert = false;
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		glLineWidth(0.1f);
#if defined(_USE_GLAPI_5_4) || defined(_USE_GLAPI_4_8)		   
		if(bGL_EXT_VERTEX_ARRAY) {
			extfunc->glEnableClientState(GL_VERTEX_ARRAY);
			extfunc->glVertexPointer(3, GL_FLOAT, 0, glHorizGrids);
			extfunc->glDrawArrays(GL_LINES, 0, (vert_lines + 1) * 2);
			extfunc->glDisableClientState(GL_VERTEX_ARRAY);
		} else
#endif	     
		{
			int y;
			glBegin(GL_LINES);
			for(y = 0; y < vert_lines; y++) {
				glVertex3f(glHorizGrids[y * 6],     glHorizGrids[y * 6 + 1], glHorizGrids[y * 6 + 2]);
				glVertex3f(glHorizGrids[y * 6 + 3], glHorizGrids[y * 6 + 4], glHorizGrids[y * 6 + 5]);
			}
			glEnd();
		}
	}

	if(gl_grid_vert && (horiz_pixels > 0) && (glVertGrids != NULL) && req_draw_grids_horiz) {
	  //req_draw_grids_horiz = false;
		glDisable(GL_TEXTURE_2D);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
		glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
		glLineWidth(0.5f);
#if defined(_USE_GLAPI_5_4) || defined(_USE_GLAPI_4_8)		   
		if(bGL_EXT_VERTEX_ARRAY) {
			extfunc->glEnableClientState(GL_VERTEX_ARRAY);
			extfunc->glVertexPointer(3, GL_FLOAT, 0, glVertGrids);
			extfunc->glDrawArrays(GL_LINES, 0, (horiz_pixels + 1)* 2);
			extfunc->glDisableClientState(GL_VERTEX_ARRAY);

		} else
#endif	     
		{
			int x;
			glBegin(GL_LINES);
			for(x = 0; x < (horiz_pixels + 1); x++) {
				glVertex3f(glVertGrids[x * 6],     glVertGrids[x * 6 + 1], glVertGrids[x * 6 + 2]);
				glVertex3f(glVertGrids[x * 6 + 3], glVertGrids[x * 6 + 4], glVertGrids[x * 6 + 5]);
			}
			glEnd();
		}
	}
}


void GLDrawClass::drawUpdateTexture(QImage *p)
{
	if((p != NULL)) {
#if defined(_USE_GLAPI_QT5_4)   
		if(uVramTextureID->isCreated()) {
	  		uVramTextureID->destroy();
			uVramTextureID->create();
		}
		uVramTextureID->setData(*p);
#else
		if(uVramTextureID != 0) {
	  		this->deleteTexture(uVramTextureID);
		}
		uVramTextureID = this->bindTexture(*p);
#endif
	}
//#ifdef _USE_OPENCL
}

#if defined(USE_BUTTON)
void GLDrawClass::updateButtonTexture(void)
{
	int i;
   	QImage *img;
   	QPainter *painter;
	QColor col;
	QRect rect;
	QPen *pen;
	if(button_updated) return;
	col.setRgb(0, 0, 0, 255);
	pen = new QPen(col);
	for(i = 0; i < MAX_BUTTONS; i++) {
		img = new QImage(buttons[i].width, buttons[i].height, QImage::Format_RGB32);
		painter = new QPainter(img);
		//painter->setRenderHint(QPainter::Antialiasing, true);
		col.setRgb(255, 255, 255, 255);
		painter->fillRect(0, 0, buttons[i].width, buttons[i].height, col);
		//painter->setPen(pen);
		rect.setWidth(buttons[i].width);
		rect.setHeight(buttons[i].height);
		rect.setX(0);
		rect.setY(0);
		painter->drawText(rect, Qt::AlignCenter, QString::fromUtf8(buttons[i].caption));
#if defined(_USE_GLAPI_QT5_4)   
		if(uBitmapTextureID->isCreated()) {
	  		uButtonTextureID[i]->destroy();
			uButtonTextureID[i]->create();
		} else {
			uButtonTextureID[i]->create();
		}
		uButtonTextureID[i]->setData(*img);
#else
		if(uButtonTextureID[i] != 0) {
	  		this->deleteTexture(uButtonTextureID[i]);
		}
		uButtonTextureID[i] = this->bindTexture(*img);;
#endif
		delete painter;
		delete img;
	}
	delete pen;
	button_updated = true;
}
#endif

#if defined(USE_BITMAP)
void GLDrawClass::uploadBitmapTexture(QImage *p)
{
	int i;
	if(p == NULL) return;
	if(!bitmap_uploaded) {
#if defined(_USE_GLAPI_QT5_4)   
		if(uBitmapTextureID->isCreated()) {
	  		uBitmapTextureID->destroy();
			uBitmapTextureID->create();
		}
		uBitmapTextureID->setData(*p);
#else
		if(uBitmapTextureID != 0) {
	  		this->deleteTexture(uBitmapTextureID);
		}
		uBitmapTextureID = this->bindTexture(*p);
#endif	   
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
#if defined(USE_BUTTON)
	h = height;
	w = width;
	ratio = (double)width / (double)height;
#else
	switch(config.stretch_type) {
	case 0: // Dot by Dot
		//ratio = (double)SCREEN_WIDTH / (double)SCREEN_HEIGHT;
# ifdef USE_SCREEN_ROTATE
		if(config.rotate_type) {
			ratio =  (double)p_emu->get_screen_height_aspect() / (double)p_emu->get_screen_width_aspect();
			h = (int)(ww / ratio);
			w = (int)(hh * ratio);
		} else
# endif	   
		{
			ratio =  (double)p_emu->get_screen_width_aspect() / (double)p_emu->get_screen_height_aspect();
			h = (int)(ww / ratio);
			w = (int)(hh * ratio);
		}
		break;
	case 1: // Keep Aspect
# ifdef USE_SCREEN_ROTATE
		if(config.rotate_type) {
			ratio = (double)WINDOW_HEIGHT_ASPECT / (double)WINDOW_WIDTH_ASPECT;
			h = (int)(ww / ratio);
			w = (int)(hh * ratio);
		} else
# endif	   
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
#endif   
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
	screen_width  = ((GLfloat) draw_width / (GLfloat)(this->width())); 
	screen_height = ((GLfloat) draw_height / (GLfloat)(this->height()));
	crt_flag = true;

	req_draw_grids_horiz = true;
	req_draw_grids_vert = true;
	
	if(draw_width  < ((horiz_pixels * 4) / 2)) req_draw_grids_horiz = false;
	if(draw_height < ((vert_lines   * 2) / 2))   req_draw_grids_vert = false;
	{
		int i;
		GLfloat yf;
		GLfloat xf;
		GLfloat delta;
		
		yf = -screen_height;
		delta = (2.0f * screen_height) / (float)vert_lines;
		yf = yf - delta * 0.75f;
		if(glHorizGrids != NULL) {
			if(vert_lines > SCREEN_HEIGHT) vert_lines = SCREEN_HEIGHT;
			for(i = 0; i < (vert_lines + 1) ; i++) {
				glHorizGrids[i * 6]     = -screen_width; // XBegin
				glHorizGrids[i * 6 + 3] = +screen_width; // XEnd
				glHorizGrids[i * 6 + 1] = yf; // YBegin
				glHorizGrids[i * 6 + 4] = yf; // YEnd
				glHorizGrids[i * 6 + 2] = 0.1f; // ZBegin
				glHorizGrids[i * 6 + 5] = 0.1f; // ZEnd
				yf = yf + delta;
			}
#if 0			
			for(; i < (SCREEN_HEIGHT + 2); i++) {
				glHorizGrids[i * 6]     = -1.5f; // XBegin
				glHorizGrids[i * 6 + 3] = -1.5f; // XEnd
				glHorizGrids[i * 6 + 1] = -1.5f; // YBegin
				glHorizGrids[i * 6 + 4] = -1.5f; // YEnd
				glHorizGrids[i * 6 + 2] = -99.0f; // ZBegin
				glHorizGrids[i * 6 + 5] = -99.0f; // ZEnd
			}
#endif			
		}
		xf = -screen_width; 
		delta = (2.0f * screen_width) / (float)horiz_pixels;
		xf = xf - delta * 0.75f;
		if(glVertGrids != NULL) {
			if(horiz_pixels > SCREEN_WIDTH) horiz_pixels = SCREEN_WIDTH;
			for(i = 0; i < (horiz_pixels + 1) ; i++) {
				glVertGrids[i * 6]     = xf; // XBegin
				glVertGrids[i * 6 + 3] = xf; // XEnd
				glVertGrids[i * 6 + 1] = -screen_height; // YBegin
				glVertGrids[i * 6 + 4] =  screen_height; // YEnd
				glVertGrids[i * 6 + 2] = 0.1f; // ZBegin
				glVertGrids[i * 6 + 5] = 0.1f; // ZEnd
				xf = xf + delta;
			}
#if 0			
			for(; i < (SCREEN_WIDTH + 2); i++) {
				glVertGrids[i * 6]     = -1.5f; // XBegin
				glVertGrids[i * 6 + 3] = -1.5f; // XEnd
				glVertGrids[i * 6 + 1] = -1.5f; // YBegin
				glVertGrids[i * 6 + 4] = -1.5f; // YEnd
				glVertGrids[i * 6 + 2] = -99.0f; // ZBegin
				glVertGrids[i * 6 + 5] = -99.0f; // ZEnd
			}
#endif			
		}
	}

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
	GLfloat TexCoords[4][2];
	GLfloat Vertexs[4][3];
	int i;
	if(!crt_flag) return;
	if(p_emu != NULL) {
		if(imgptr == NULL) return;
		drawUpdateTexture(imgptr);
		crt_flag = false;
	}
#if defined(_USE_GLAPI_5_4)
	TexCoords[0][0] = TexCoords[3][0] = 0.0f; // Xbegin
	TexCoords[0][1] = TexCoords[1][1] = 0.0f; // Ybegin
   
	TexCoords[2][0] = TexCoords[1][0] = 1.0f; // Xend
	TexCoords[2][1] = TexCoords[3][1] = 1.0f; // Yend
#else
	TexCoords[0][0] = TexCoords[3][0] = 0.0f; // Xbegin
	TexCoords[0][1] = TexCoords[1][1] = 1.0f; // Ybegin
   
	TexCoords[2][0] = TexCoords[1][0] = 1.0f; // Xend
	TexCoords[2][1] = TexCoords[3][1] = 0.0f; // Yend
#endif
	Vertexs[0][2] = Vertexs[1][2] = Vertexs[2][2] = Vertexs[3][2] = -0.0f; // BG
	Vertexs[0][0] = Vertexs[3][0] = -screen_width; // Xbegin
	Vertexs[0][1] = Vertexs[1][1] =  screen_height;  // Yend
	Vertexs[2][0] = Vertexs[1][0] =  screen_width; // Xend
	Vertexs[2][1] = Vertexs[3][1] = -screen_height; // Ybegin

	
	glPushAttrib(GL_TEXTURE_BIT);
	glPushAttrib(GL_TRANSFORM_BIT);
	glPushAttrib(GL_ENABLE_BIT);
#ifdef _USE_OPENCL
	//    InitContextCL();   
#endif

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

#ifdef USE_BITMAP
	glEnable(GL_TEXTURE_2D);
# if defined(_USE_GLAPI_QT_5_4)
	if(uBitmapTextureID->isCreated()) {
		uBitmapTextureID->bind();
# else	   
	if(uBitmapTextureID != 0) {
		glBindTexture(GL_TEXTURE_2D, uBitmapTextureID);	   
# endif
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
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
# if defined(_USE_GLAPI_QT5_4)
		uBitmapTextureID->release();
	}
# else
		glBindTexture(GL_TEXTURE_2D, 0);
	}
# endif	   

	glDisable(GL_TEXTURE_2D);
#endif	
#ifdef USE_SCREEN_ROTATE   
	glPushMatrix();
#endif   

#ifdef USE_BITMAP
# if defined(_USE_GLAPI_QT_5_4)
	if(uBitmapTextureID->isCreated()) {
		Vertexs[0][2] = Vertexs[1][2] = Vertexs[2][2] = Vertexs[3][2] = 0.1f; // BG
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
# else   
	if(uBitmapTextureID != 0) {
		Vertexs[0][2] = Vertexs[1][2] = Vertexs[2][2] = Vertexs[3][2] = 0.1f; // BG
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
# endif 
#endif
	/*
	 * VRAMの表示:テクスチャ貼った四角形
	 */
	glEnable(GL_TEXTURE_2D);
# if defined(_USE_GLAPI_QT5_4)
	uVramTextureID->bind();
# else
	glBindTexture(GL_TEXTURE_2D, uVramTextureID);
# endif	   
	
	if(!smoosing) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	glBegin(GL_POLYGON);
	glColor4f(1.0f , 1.0f, 1.0f, 1.0f);
	glTexCoord2f(TexCoords[0][0], TexCoords[0][1]);
	glVertex3f(Vertexs[0][0], Vertexs[0][1], Vertexs[0][2]);
	
	glTexCoord2f(TexCoords[1][0], TexCoords[1][1]);
	glVertex3f(Vertexs[1][0], Vertexs[1][1], Vertexs[1][2]);
	 
	glTexCoord2f(TexCoords[2][0], TexCoords[2][1]);
	glVertex3f(Vertexs[2][0], Vertexs[2][1], Vertexs[2][2]);
	      
	glTexCoord2f(TexCoords[3][0], TexCoords[3][1]);
	glVertex3f(Vertexs[3][0], Vertexs[3][1], Vertexs[3][2]);
	glEnd();
	
# if defined(_USE_GLAPI_QT5_4)
	uVramTextureID->release();
# else
	glBindTexture(GL_TEXTURE_2D, 0);
# endif	   

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	if(set_brightness) {
		glEnable(GL_BLEND);
		glColor3f(fBrightR , fBrightG, fBrightB);
		glBlendFunc(GL_ZERO, GL_SRC_COLOR);
    		//    glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
		glBegin(GL_POLYGON);
		glVertex3f(Vertexs[0][0], Vertexs[0][1], Vertexs[0][2]);
		glVertex3f(Vertexs[1][0], Vertexs[1][1], Vertexs[1][2]);
		glVertex3f(Vertexs[2][0], Vertexs[2][1], Vertexs[2][2]);
		glVertex3f(Vertexs[3][0], Vertexs[3][1], Vertexs[3][2]);
		glEnd();
		glBlendFunc(GL_ONE, GL_ZERO);
		glDisable(GL_BLEND);
	}
	drawGrids();
#if defined(USE_BUTTON)
	updateButtonTexture();
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	for(i = 0; i < MAX_BUTTONS; i++) {
# if defined(_USE_GLAPI_QT5_4)
		uButtonTextureID[i]->bind();
# else
		glBindTexture(GL_TEXTURE_2D, uButtonTextureID[i]);
# endif	   
		Vertexs[0][2] = Vertexs[1][2] = Vertexs[2][2] = Vertexs[3][2] = 0.2f; // BG
		Vertexs[0][0] = Vertexs[3][0] = fButtonX[i]; // Xbegin
		Vertexs[0][1] = Vertexs[1][1] = fButtonY[i] ;  // Yend
		Vertexs[2][0] = Vertexs[1][0] = fButtonX[i] + fButtonWidth[i]; // Xend
		Vertexs[2][1] = Vertexs[3][1] = fButtonY[i] - fButtonHeight[i]; // Ybegin
	   
		glBegin(GL_POLYGON);
		glColor4f(1.0f , 1.0f, 1.0f, 1.0f);
		glTexCoord2f(TexCoords[0][0], TexCoords[0][1]);
		glVertex3f(Vertexs[0][0], Vertexs[0][1], Vertexs[0][2]);
	
		glTexCoord2f(TexCoords[1][0], TexCoords[1][1]);
		glVertex3f(Vertexs[1][0], Vertexs[1][1], Vertexs[1][2]);
	 
		glTexCoord2f(TexCoords[2][0], TexCoords[2][1]);
		glVertex3f(Vertexs[2][0], Vertexs[2][1], Vertexs[2][2]);
	      
		glTexCoord2f(TexCoords[3][0], TexCoords[3][1]);
		glVertex3f(Vertexs[3][0], Vertexs[3][1], Vertexs[3][2]);
		glEnd();
# if defined(_USE_GLAPI_QT5_4)
		uButtonTextureID[i]->release();
# else
		glBindTexture(GL_TEXTURE_2D, 0);
# endif	   
	}
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
#endif   
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
}

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

GLDrawClass::GLDrawClass(QWidget *parent)
#if defined(_USE_GLAPI_QT5_4)   
  : QOpenGLWidget(parent, Qt::Widget)
#else
  : QGLWidget(parent)
#endif

{
#if defined(_USE_GLAPI_QT5_4)   
	uVramTextureID = new QOpenGLTexture(QOpenGLTexture::Target2D);
#else
	uVramTextureID = 0;
#endif
	imgptr = NULL;
	p_emu = NULL;
	extfunc = NULL;
#ifdef USE_BITMAP
# if defined(_USE_GLAPI_QT5_4)   
	uBitmapTextureID = new QOpenGLTexture(QOpenGLTexture::Target2D);
# else
	uBitmapTextureID = 0;
# endif
	bitmap_uploaded = false;
#endif
#ifdef USE_BUTTON
	int i;
	for(i = 0; i < MAX_BUTTONS; i++) {
# if defined(_USE_GLAPI_QT5_4)   
		uButtonTextureID[i] = new QOpenGLTexture(QOpenGLTexture::Target2D);
# else	   
		uButtonTextureID[i] = 0;
# endif	   
		fButtonX[i] = -1.0 + (float)(buttons[i].x * 2) / (float)SCREEN_WIDTH;
		fButtonY[i] = 1.0 - (float)(buttons[i].y * 2) / (float)SCREEN_HEIGHT;
		fButtonWidth[i] = (float)(buttons[i].width * 2) / (float)SCREEN_WIDTH;
		fButtonHeight[i] = (float)(buttons[i].height * 2) / (float)SCREEN_HEIGHT;
	}
	button_updated = false;
#endif
	req_draw_grids_horiz = false;
	req_draw_grids_vert = false;
        fBrightR = 1.0; // 輝度の初期化
        fBrightG = 1.0;
        fBrightB = 1.0;
	set_brightness = false;
	crt_flag = false;
	smoosing = false;
	
	gl_grid_horiz = false;
	gl_grid_vert = false;
	glVertGrids = NULL;
	glHorizGrids = NULL;

	vert_lines = SCREEN_HEIGHT;
	horiz_pixels = SCREEN_WIDTH;
	enable_mouse = true;
	screen_width = 1.0;
	screen_height = 1.0;
#ifdef _USE_OPENCL
        bInitCL = false;
        nCLGlobalWorkThreads = 10;
        bCLSparse = false; // true=Multi threaded CL,false = Single Thread.
	nCLPlatformNum = 0;
	nCLDeviceNum = 0;
	bCLInteropGL = false;
    //bCLDirectMapping = false;
#endif
        this->setFocusPolicy(Qt::StrongFocus);
}

GLDrawClass::~GLDrawClass()
{
#if defined(_USE_GLAPI_QT5_4)   
	if(uVramTextureID->isCreated()) {
  		uVramTextureID->destroy();
	}
	delete uVramTextureID;
#else
	this->deleteTexture(uVramTextureID);
#endif     
#ifdef USE_BITMAP
# if defined(_USE_GLAPI_QT5_4)   
	if(uBitmapTextureID->isCreated()) {
  		uBitmapTextureID->destroy();
	}
	delete uBitmapTextureID;
# else   
	if(uBitmapTextureID != 0) {
  		this->deleteTexture(uBitmapTextureID);
	}
# endif 
#endif
#ifdef USE_BUTTON
	int i;
	for(i = 0; i < MAX_BUTTONS; i++) {
# if defined(_USE_GLAPI_QT5_4)   
		if(uButtonTextureID[i]->isCreated()) uButtonTextureID[i]->destroy();
		delete uButtonTextureID[i];
# else
		if(uButtonTextureID[i] != 0) this->deleteTexture(uButtonTextureID[i]);
# endif
	}
#endif   
	if(glVertGrids != NULL) free(glVertGrids);
	if(glHorizGrids != NULL) free(glHorizGrids);
	if(extfunc != NULL) delete extfunc;
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

