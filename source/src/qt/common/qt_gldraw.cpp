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

#if defined(_USE_GLAPI_QT5_4)
void GLDrawClass::drawGridsHorizonal(void)
{
	//req_draw_grids_vert = false;
	glLineWidth(0.1f);
	if(bGL_EXT_VERTEX_ARRAY) {
		extfunc->glEnableClientState(GL_VERTEX_ARRAY);
		extfunc->glVertexPointer(3, GL_FLOAT, 0, glHorizGrids);
		extfunc->glDrawArrays(GL_LINES, 0, (vert_lines + 1) * 2);
		extfunc->glDisableClientState(GL_VERTEX_ARRAY);
	} else {
		int y;
		glBegin(GL_LINES);
		for(y = 0; y < vert_lines; y++) {
			glVertex3f(glHorizGrids[y * 6],     glHorizGrids[y * 6 + 1], glHorizGrids[y * 6 + 2]);
			glVertex3f(glHorizGrids[y * 6 + 3], glHorizGrids[y * 6 + 4], glHorizGrids[y * 6 + 5]);
		}
		glEnd();
	}
}

void GLDrawClass::drawGridsVertical(void)
{
	  //req_draw_grids_horiz = false;
		glLineWidth(0.5f);
		if(bGL_EXT_VERTEX_ARRAY) {
			extfunc->glEnableClientState(GL_VERTEX_ARRAY);
			extfunc->glVertexPointer(3, GL_FLOAT, 0, glVertGrids);
			extfunc->glDrawArrays(GL_LINES, 0, (horiz_pixels + 1)* 2);
			extfunc->glDisableClientState(GL_VERTEX_ARRAY);
		} else {
			int x;
			glBegin(GL_LINES);
			for(x = 0; x < (horiz_pixels + 1); x++) {
				glVertex3f(glVertGrids[x * 6],     glVertGrids[x * 6 + 1], glVertGrids[x * 6 + 2]);
				glVertex3f(glVertGrids[x * 6 + 3], glVertGrids[x * 6 + 4], glVertGrids[x * 6 + 5]);
			}
			glEnd();
		}
}

#else
void GLDrawClass::drawGridsHorizonal(void)
{
	//req_draw_grids_vert = false;
	int y;
	glLineWidth(0.1f);
	glBegin(GL_LINES);
	for(y = 0; y < vert_lines; y++) {
		glVertex3f(glHorizGrids[y * 6],     glHorizGrids[y * 6 + 1], glHorizGrids[y * 6 + 2]);
		glVertex3f(glHorizGrids[y * 6 + 3], glHorizGrids[y * 6 + 4], glHorizGrids[y * 6 + 5]);
	}
	glEnd();
}

void GLDrawClass::drawGridsVertical(void)
{
	  //req_draw_grids_horiz = false;
	int x;
	glLineWidth(0.5f);
	glBegin(GL_LINES);
	for(x = 0; x < (horiz_pixels + 1); x++) {
		glVertex3f(glVertGrids[x * 6],     glVertGrids[x * 6 + 1], glVertGrids[x * 6 + 2]);
		glVertex3f(glVertGrids[x * 6 + 3], glVertGrids[x * 6 + 4], glVertGrids[x * 6 + 5]);
	}
	glEnd();
}

#endif

void GLDrawClass::drawGrids(void)
{
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
	if(gl_grid_horiz && (vert_lines > 0) && (glHorizGrids != NULL) && req_draw_grids_vert) {
		this->drawGridsHorizonal();
	}

	if(gl_grid_vert && (horiz_pixels > 0) && (glVertGrids != NULL) && req_draw_grids_horiz) {
		this->drawGridsVertical();
	}
}
void GLDrawClass::adjustBrightness()
{
	glEnable(GL_BLEND);
	glColor3f(fBrightR , fBrightG, fBrightB);
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	//    glBlendFunc(GL_ZERO, GL_SRC_ALPHA);
	glBegin(GL_POLYGON);
	glVertex3f(ScreenVertexs[0][0], ScreenVertexs[0][1], ScreenVertexs[0][2]);
	glVertex3f(ScreenVertexs[1][0], ScreenVertexs[1][1], ScreenVertexs[1][2]);
	glVertex3f(ScreenVertexs[2][0], ScreenVertexs[2][1], ScreenVertexs[2][2]);
	glVertex3f(ScreenVertexs[3][0], ScreenVertexs[3][1], ScreenVertexs[3][2]);
	glEnd();
	glBlendFunc(GL_ONE, GL_ZERO);
	glDisable(GL_BLEND);
}

#if defined(USE_BUTTON)
void GLDrawClass::drawButtons()
{
	int i;
	GLfloat Vertexs[4][3];
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
}
#endif
#if defined(_USE_GLAPI_QT5_4)

# ifdef USE_BITMAP
void GLDrawClass::drawBitmapTexture(void)
{
	glEnable(GL_TEXTURE_2D);
	if(uBitmapTextureID->isCreated()) {
		uBitmapTextureID->bind();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBegin(GL_POLYGON);
		glTexCoord2f(TexCoords[0][0], TexCoords[0][1]);
		glVertex3f(BitmapVertexs[0][0], BitmapVertexs[0][1], BitmapVertexs[0][2]);
	
		glTexCoord2f(TexCoords[1][0], TexCoords[1][1]);
		glVertex3f(BitmapVertexs[1][0], BitmapVertexs[1][1], BitmapVertexs[1][2]);
	 
		glTexCoord2f(TexCoords[2][0], TexCoords[2][1]);
		glVertex3f(BitmapVertexs[2][0], BitmapVertexs[2][1], BitmapVertexs[2][2]);
		
		glTexCoord2f(TexCoords[3][0], TexCoords[3][1]);
		glVertex3f(BitmapVertexs[3][0], BitmapVertexs[3][1], BitmapVertexs[3][2]);
		glEnd();
		uBitmapTextureID->release();
	}
	glDisable(GL_TEXTURE_2D);
}
# endif	

void GLDrawClass::drawScreenTexture(void)
{
# ifdef USE_BITMAP
	if(uBitmapTextureID->isCreated()) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
# endif
	if(uVramTextureID->isCreated()) {
		glEnable(GL_TEXTURE_2D);
		uVramTextureID->bind();
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
		glVertex3f(ScreenVertexs[0][0], ScreenVertexs[0][1], ScreenVertexs[0][2]);
	
		glTexCoord2f(TexCoords[1][0], TexCoords[1][1]);
		glVertex3f(ScreenVertexs[1][0], ScreenVertexs[1][1], ScreenVertexs[1][2]);
		
		glTexCoord2f(TexCoords[2][0], TexCoords[2][1]);
		glVertex3f(ScreenVertexs[2][0], ScreenVertexs[2][1], ScreenVertexs[2][2]);
		
		glTexCoord2f(TexCoords[3][0], TexCoords[3][1]);
		glVertex3f(ScreenVertexs[3][0], ScreenVertexs[3][1], ScreenVertexs[3][2]);
		glEnd();
	
		uVramTextureID->release();
		glDisable(GL_TEXTURE_2D);
	}
}
#else // Not _GLAPI_QT_5_4
# ifdef USE_BITMAP
void GLDrawClass::drawBitmapTexture(void)
{
	glEnable(GL_TEXTURE_2D);
	if(uBitmapTextureID != 0) {
		glBindTexture(GL_TEXTURE_2D, uBitmapTextureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glBegin(GL_POLYGON);
		glTexCoord2f(TexCoords[0][0], TexCoords[0][1]);
		glVertex3f(BitmapVertexs[0][0], BitmapVertexs[0][1], BitmapVertexs[0][2]);
	
		glTexCoord2f(TexCoords[1][0], TexCoords[1][1]);
		glVertex3f(BitmapVertexs[1][0], BitmapVertexs[1][1], BitmapVertexs[1][2]);
	 
		glTexCoord2f(TexCoords[2][0], TexCoords[2][1]);
		glVertex3f(BitmapVertexs[2][0], BitmapVertexs[2][1], BitmapVertexs[2][2]);
		
		glTexCoord2f(TexCoords[3][0], TexCoords[3][1]);
		glVertex3f(BitmapVertexs[3][0], BitmapVertexs[3][1], BitmapVertexs[3][2]);
		glEnd();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	glDisable(GL_TEXTURE_2D);
}
# endif

void GLDrawClass::drawScreenTexture(void)
{
# ifdef USE_BITMAP
	if(uBitmapTextureID != 0) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
# endif
	if(uVramTextureID != 0) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, uVramTextureID);
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
		glVertex3f(ScreenVertexs[0][0], ScreenVertexs[0][1], ScreenVertexs[0][2]);
	
		glTexCoord2f(TexCoords[1][0], TexCoords[1][1]);
		glVertex3f(ScreenVertexs[1][0], ScreenVertexs[1][1], ScreenVertexs[1][2]);
		
		glTexCoord2f(TexCoords[2][0], TexCoords[2][1]);
		glVertex3f(ScreenVertexs[2][0], ScreenVertexs[2][1], ScreenVertexs[2][2]);
		
		glTexCoord2f(TexCoords[3][0], TexCoords[3][1]);
		glVertex3f(ScreenVertexs[3][0], ScreenVertexs[3][1], ScreenVertexs[3][2]);
		glEnd();
	
		glBindTexture(GL_TEXTURE_2D, 0);
		glDisable(GL_TEXTURE_2D);
	}
}
#endif

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
		if( uButtonTextureID[i]->isCreated()) {
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
	int w, h;
#if !defined(USE_BUTTON)
# ifdef USE_SCREEN_ROTATE
	if(config.rotate_type) {
		int tmp;
		tmp = width;
		width = height;
		height = tmp;
	}
# endif
#endif
	glViewport(0, 0, width, height);
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
		}
	}
	ScreenVertexs[0][2] = ScreenVertexs[1][2] = ScreenVertexs[2][2] = ScreenVertexs[3][2] = -0.0f; // BG
	ScreenVertexs[0][0] = ScreenVertexs[3][0] = -screen_width; // Xbegin
	ScreenVertexs[0][1] = ScreenVertexs[1][1] =  screen_height;  // Yend
	ScreenVertexs[2][0] = ScreenVertexs[1][0] =  screen_width; // Xend
	ScreenVertexs[2][1] = ScreenVertexs[3][1] = -screen_height; // Ybegin
#if defined(USE_BITMAP)	
	BitmapVertexs[0][2] = BitmapVertexs[1][2] = BitmapVertexs[2][2] = BitmapVertexs[3][2] = -0.1f; // BG
	BitmapVertexs[0][0] = BitmapVertexs[3][0] = -screen_width; // Xbegin
	BitmapVertexs[0][1] = BitmapVertexs[1][1] =  screen_height;  // Yend
	BitmapVertexs[2][0] = BitmapVertexs[1][0] =  screen_width; // Xend
	BitmapVertexs[2][1] = BitmapVertexs[3][1] = -screen_height; // Ybegin
#endif
	
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
	if(!crt_flag) return;
	if(p_emu != NULL) {
		if(imgptr == NULL) return;
		drawUpdateTexture(imgptr);
		crt_flag = false;
	}
	
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
	drawBitmapTexture();
#endif	
#ifdef USE_SCREEN_ROTATE   
	glPushMatrix();
#endif   
	/*
	 * VRAMの表示:テクスチャ貼った四角形
	 */
	drawScreenTexture();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	if(set_brightness) {
		adjustBrightness();
	}
	drawGrids();
#if defined(USE_BUTTON)
	drawButtons();
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
	
	ScreenVertexs[0][2] = ScreenVertexs[1][2] = ScreenVertexs[2][2] = ScreenVertexs[3][2] = -0.0f; // BG
	ScreenVertexs[0][0] = ScreenVertexs[3][0] = -1.0f; // Xbegin
	ScreenVertexs[0][1] = ScreenVertexs[1][1] =  1.0f;  // Yend
	ScreenVertexs[2][0] = ScreenVertexs[1][0] =  1.0f; // Xend
	ScreenVertexs[2][1] = ScreenVertexs[3][1] = -1.0f; // Ybegin
#if defined(USE_BITMAP)	
	BitmapVertexs[0][2] = BitmapVertexs[1][2] = BitmapVertexs[2][2] = BitmapVertexs[3][2] = -0.1f; // BG
	BitmapVertexs[0][0] = BitmapVertexs[3][0] = -1.0f; // Xbegin
	BitmapVertexs[0][1] = BitmapVertexs[1][1] =  1.0f;  // Yend
	BitmapVertexs[2][0] = BitmapVertexs[1][0] =  1.0f; // Xend
	BitmapVertexs[2][1] = BitmapVertexs[3][1] = -1.0f; // Ybegin
#endif
#if defined(_USE_GLAPI_QT5_4)
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
	button_drawn = false;
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

