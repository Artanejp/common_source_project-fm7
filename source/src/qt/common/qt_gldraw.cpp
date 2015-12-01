/*
 * qt_gldraw.cpp
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * 
 */


#include "emu.h"


#include <QtGui>
#if defined(MAX_BUTTONS) || defined(ONE_BOARD_MICRO_COMPUTER)
#include <QColor>
#include <QPainter>
#include <QPen>
#include <QRect>
#endif
//#include <SDL/SDL.h>
#if defined(_WINDOWS) || defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
#include <GL/gl.h>
#include <GL/glext.h>
#else
# if !defined(_USE_GLAPI_QT5_4) || !defined(_USE_GLAPI_QT5_1)  
#  include <GL/glx.h>
#  include <GL/glxext.h>
# endif
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



void GLDrawClass::drawGridsMain(QOpenGLShaderProgram *prg,
								QOpenGLVertexArrayObject *vp,
								QOpenGLBuffer *bp,
								int number,
								GLfloat lineWidth,
								QVector4D color)
{
		extfunc->glDisable(GL_TEXTURE_2D);
		extfunc->glDisable(GL_BLEND);
		vp->bind();
		bp->bind();
		prg->bind();
		extfunc->glLineWidth(lineWidth);
		prg->setUniformValue("color", color);
		prg->enableAttributeArray("vertex");
# ifdef USE_SCREEN_ROTATE
		if(config.rotate_type) {
			prg->setUniformValue("rotate", GL_TRUE);
		} else {
			prg->setUniformValue("rotate", GL_FALSE);
		}
#else		
		prg->setUniformValue("rotate", GL_FALSE);
#endif	   
		extfunc->glEnableVertexAttribArray(0);
		extfunc->glEnable(GL_VERTEX_ARRAY);
   		extfunc->glDrawArrays(GL_LINES, 0, (number + 1) * 2);
		bp->release();
		vp->release();
		prg->release();
}

void GLDrawClass::drawGridsHorizonal(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	drawGridsMain(grids_shader_horizonal,
				  vertex_grid_horizonal,
				  buffer_grid_horizonal,
				  vert_lines,
				  0.25f,
				  c);
}

void GLDrawClass::drawGridsVertical(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	drawGridsMain(grids_shader_vertical,
				  vertex_grid_vertical,
				  buffer_grid_vertical,
				  horiz_pixels,
				  0.5f,
				  c);
}


void GLDrawClass::drawGrids(void)
{
	gl_grid_horiz = config.opengl_scanline_horiz;
	gl_grid_vert  = config.opengl_scanline_vert;
	extfunc->glDisable(GL_TEXTURE_2D);
	extfunc->glDisable(GL_BLEND);
	if(gl_grid_horiz && (vert_lines > 0)) {
		this->drawGridsHorizonal();
	}
#if defined(USE_VERTICAL_PIXEL_LINES)		
	if(gl_grid_vert && (horiz_pixels > 0)) {
		this->drawGridsVertical();
	}
#endif	
}

#if defined(MAX_BUTTONS)
void GLDrawClass::drawButtons()
{
	int i;
	//updateButtonTexture();
	for(i = 0; i < MAX_BUTTONS; i++) {
		QVector4D c;
		c = QVector4D(1.0, 1.0, 1.0, 1.0);
		drawMain(button_shader[i], vertex_button[i],
				 buffer_button_vertex[i], uButtonTextureID[i],
				 c, false);
	}
}
#endif
#if defined(_USE_GLAPI_QT5_4)

# ifdef ONE_BOARD_MICRO_COMPUTER
void GLDrawClass::drawBitmapTexture(void)
{
	extfunc->glEnable(GL_TEXTURE_2D);
	if(uBitmapTextureID->isCreated()) {
		uBitmapTextureID->bind();

		extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		{
			QVector4D c;
			c = QVector4D(1.0, 1.0, 1.0, 1.0);
			bitmap_shader->setUniformValue("color", c);
		}
		if(vertex_bitmap->isCreated()) {
			main_shader->setUniformValue("texture", 0);
			//main_shader->setUniformValue("v_texcoord", *texture_texcoord);
			extfunc->glClear(GL_COLOR_BUFFER_BIT);
			vertex_bitmap->bind();
			extfunc->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			vertex_bitmap->release();
		}
		uBitmapTextureID->release();
	}
	extfunc->glDisable(GL_TEXTURE_2D);
}
# endif	

void GLDrawClass::drawScreenTexture(void)
{
# ifdef ONE_BOARD_MICRO_COMPUTER
	if(uBitmapTextureID->isCreated()) {
		extfunc->glEnable(GL_BLEND);
		extfunc->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
# endif
	smoosing = config.use_opengl_filters;
	if(uVramTextureID->isCreated()) {
		extfunc->glEnable(GL_TEXTURE_2D);
		uVramTextureID->bind();
#if 1
		main_shader->bind();
#endif		
		if(!smoosing) {
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		} else {
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		if(vertex_screen->isCreated()) {
			//extfunc->glClear(GL_COLOR_BUFFER_BIT);
			vertex_screen->bind();
			main_shader->setUniformValue("texture", uVramTextureID->textureId());
			int vertex_loc = main_shader->attributeLocation("position");
			main_shader->enableAttributeArray(vertex_loc);
			main_shader->setAttributeArray(vertex_loc, ScreenVertexs, 4);
			
			int texcoord_loc = main_shader->attributeLocation("a_texcoord");
#if defined(ONE_BOARD_MICRO_COMPUTER) || defined(MAX_BUTTONS)
			QVector3D ch = QVector3D(0.0f, 0.0f, 0.0f);
			main_shader->setUniformValue("chromakey", ch);
#endif			
			main_shader->enableAttributeArray(texcoord_loc);
			main_shader->setAttributeArray(texcoord_loc, TexCoords, 4);
		
			extfunc->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			main_shader->disableAttribArray(texcoord_loc);
			main_shader->disableAttribArray(vertex_loc);
			vertex_acreen->release();
		}
		uVramTextureID->release();
		extfunc->glDisable(GL_TEXTURE_2D);
#if 1
		main_shader->release();
#endif		
	}
}
#else // Not _GLAPI_QT_5_4
void GLDrawClass::drawMain(QOpenGLShaderProgram *prg,
						   QOpenGLVertexArrayObject *vp,
						   QOpenGLBuffer *bp,
						   GLuint texid,
						   QVector4D color,
						   bool f_smoosing,
						   bool use_chromakey,
						   QVector3D chromakey)
						   
{
	if(texid != 0) {
		extfunc->glEnable(GL_TEXTURE_2D);
		vp->bind();
		bp->bind();
		prg->bind();
		extfunc->glActiveTexture(GL_TEXTURE0);
		extfunc->glBindTexture(GL_TEXTURE_2D, texid);
		if(!f_smoosing) {
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		} else {
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		}
		prg->setUniformValue("a_texture", 0);
		prg->setUniformValue("color", color);
#if defined(ONE_BOARD_MICRO_COMPUTER) || defined(MAX_BUTTONS)
		if(use_chromakey) {
			prg->setUniformValue("chromakey", chromakey);
		}
#endif			
# ifdef USE_SCREEN_ROTATE
			if(config.rotate_type) {
				prg->setUniformValue("rotate", GL_TRUE);
			} else {
				prg->setUniformValue("rotate", GL_FALSE);
			}
#else		
			prg->setUniformValue("rotate", GL_FALSE);
#endif	   
		prg->enableAttributeArray("texcoord");
		prg->enableAttributeArray("vertex");
		extfunc->glEnableVertexAttribArray(0);
		extfunc->glEnableVertexAttribArray(1);
		extfunc->glEnable(GL_VERTEX_ARRAY);

		extfunc->glDrawArrays(GL_POLYGON, 0, 4);
		bp->release();
		vp->release();
		
		prg->release();
		extfunc->glBindTexture(GL_TEXTURE_2D, 0);
		extfunc->glDisable(GL_TEXTURE_2D);
	}
}

# ifdef ONE_BOARD_MICRO_COMPUTER
void GLDrawClass::drawBitmapTexture(void)
{
	QVector4D c;
	c = QVector4D(1.0, 1.0, 1.0, 1.0);
	drawMain(bitmap_shader, vertex_bitmap,
			 buffer_bitmap_vertex, uBitmapTextureID,
			 c, false);
}
# endif


void GLDrawClass::drawScreenTexture(void)
{
#ifdef ONE_BOARD_MICRO_COMPUTER
	if(uBitmapTextureID != 0) {
		extfunc->glEnable(GL_BLEND);
		extfunc->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
#else
	extfunc->glDisable(GL_BLEND);
#endif
	
	QVector4D color;
	smoosing = config.use_opengl_filters;
	if(set_brightness) {
		color = QVector4D(fBrightR, fBrightG, fBrightB, 1.0);
	} else {
		color = QVector4D(1.0, 1.0, 1.0, 1.0);
	}			
#if defined(ONE_BOARD_MICRO_COMPUTER) || defined(MAX_BUTTONS)
	{
		QVector3D ch = QVector3D(0.0f, 0.0f, 0.0f);
		drawMain(main_shader, vertex_screen,
				 buffer_screen_vertex, uVramTextureID,
				 color, smoosing, true, ch);
	}
#else
	{
		main_shader->setUniformValue("color", color);
		drawMain(main_shader, vertex_screen,
				 buffer_screen_vertex, uVramTextureID,
				 color, smoosing);
	}		
#endif			
#ifdef ONE_BOARD_MICRO_COMPUTER
	extfunc->glDisable(GL_BLEND);
#endif	
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
			extfunc->glBindTexture(GL_TEXTURE_2D, uVramTextureID);
			extfunc->glTexSubImage2D(GL_TEXTURE_2D, 0,
						 0, 0,
						 p->width(), p->height(),
						 GL_BGRA, GL_UNSIGNED_BYTE, p->constBits());
			extfunc->glBindTexture(GL_TEXTURE_2D, 0);
	  		//this->deleteTexture(uVramTextureID);
		} else {
			uVramTextureID = this->bindTexture(*p);
		}
#endif
	}
//#ifdef _USE_OPENCL
}

#if defined(MAX_BUTTONS)
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

#if defined(ONE_BOARD_MICRO_COMPUTER)
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
	redraw_required = true;
	bitmap_uploaded = false;
	uploadBitmapTexture(p);
}
#endif

void GLDrawClass::resizeGL(int width, int height)
{
	int side = qMin(width, height);
	double ww, hh;
	int w, h;
#if !defined(MAX_BUTTONS)
# ifdef USE_SCREEN_ROTATE
	//if(config.rotate_type) {
	//	int tmp;
	//	tmp = width;
	//	width = height;
	//	height = tmp;
	//}
# endif
#endif
	extfunc->glViewport(0, 0, width, height);
	crt_flag = true;

	req_draw_grids_horiz = true;
	req_draw_grids_vert = true;
	
	if(draw_width  < ((horiz_pixels * 4) / 2)) req_draw_grids_horiz = false;
	if(draw_height < ((vert_lines   * 2) / 2))   req_draw_grids_vert = false;
	doSetGridsHorizonal(vert_lines, true);
#if defined(USE_VERTICAL_PIXEL_LINES)		
	doSetGridsVertical(horiz_pixels, true);
#endif		
	if(vertex_screen->isCreated()) {
		{
			vertexFormat[0].x = -screen_width;
			vertexFormat[0].y = -screen_height;
	   
			vertexFormat[1].x = +screen_width;
			vertexFormat[1].y = -screen_height;
	   
			vertexFormat[2].x = +screen_width;
			vertexFormat[2].y = +screen_height;
	   
			vertexFormat[3].x = -screen_width;
			vertexFormat[3].y = +screen_height;
		}
		setNormalVAO(main_shader, vertex_screen,
					 buffer_screen_vertex,
					 vertexFormat, 4);
	}

	
#if defined(ONE_BOARD_MICRO_COMPUTER)	
	if(vertex_bitmap->isCreated()) {
		vertexBitmap[0].x = -screen_width;
		vertexBitmap[0].y = -screen_height;
	   
		vertexBitmap[1].x = +screen_width;
		vertexBitmap[1].y = -screen_height;
	   
		vertexBitmap[2].x = +screen_width;
		vertexBitmap[2].y = +screen_height;
	   
		vertexBitmap[3].x = -screen_width;
		vertexBitmap[3].y = +screen_height;
	   
		setNormalVAO(bitmap_shader, vertex_bitmap,
					 buffer_bitmap_vertex,
					 vertexBitmap, 4);
	}
#endif
#if defined(MAX_BUTTONS)
	updateButtonTexture();
	for(int i = 0; i < MAX_BUTTONS; i++) {
#if 0		
		fButtonX[i] = -1.0 + (float)(buttons[i].x * 2) / (float)SCREEN_WIDTH;
		fButtonY[i] = 1.0 - (float)(buttons[i].y * 2) / (float)SCREEN_HEIGHT;
		fButtonWidth[i] = (float)(buttons[i].width * 2) / (float)SCREEN_WIDTH;
		fButtonHeight[i] = (float)(buttons[i].height * 2) / (float)SCREEN_HEIGHT;
		VertexTexCoord_t vt[4];

		vt[0] = vertexButtons->value(i * 4);
		vt[1] = vertexButtons->value(i * 4 + 1);
		vt[2] = vertexButtons->value(i * 4 + 2);
		vt[3] = vertexButtons->value(i * 4 + 3);
		
		vt[0].x =  fButtonX[i];
		vt[0].y =  fButtonY[i];
					   
		vt[1].x =  fButtonX[i] + fButtonWidth[i];
		vt[1].y =  fButtonY[i];
		
		vt[2].x =  fButtonX[i] + fButtonWidth[i];
		vt[2].y =  fButtonY[i] - fButtonHeight[i];
						
		vt[3].x =  fButtonX[i];
		vt[3].y =  fButtonY[i] - fButtonHeight[i];
		
		vertexButtons->replace(i * 4, vt[0]);
		vertexButtons->replace(i * 4 + 1, vt[1]);
		vertexButtons->replace(i * 4 + 2, vt[2]);
		vertexButtons->replace(i * 4 + 3, vt[3]);
		
		setNormalVAO(button_shader[i], vertex_button[i],
					 buffer_button_vertex[i],
					 vt, 4);
#endif		
	}
#endif
	redraw_required = true;
	AGAR_DebugLog(AGAR_LOG_DEBUG, "ResizeGL: %dx%d", width , height);
	emit sig_resize_uibar(width, height);
}


/*
 * "Paint" Event handler
 */

void GLDrawClass::paintGL(void)
{
	int i;
	if(!crt_flag) return;
	SaveToPixmap(); // If save requested, then Save to Pixmap.
#if !defined(USE_MINIMUM_RENDERING)
	redraw_required = true;
#endif
	if(!redraw_required) return;
	if(p_emu != NULL) {
		//if(imgptr == NULL) return;
		drawUpdateTexture(imgptr);
		crt_flag = false;
	}
	if(redraw_required) {
		extfunc->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	extfunc->glEnable(GL_DEPTH_TEST);
	extfunc->glDisable(GL_BLEND);

#ifdef ONE_BOARD_MICRO_COMPUTER
	drawBitmapTexture();
#endif	
#if defined(MAX_BUTTONS)
	drawButtons();
#endif	
	/*
	 * VRAMの表示:テクスチャ貼った四角形
	 */
	drawScreenTexture();
	emit sig_draw_timing(false);
	extfunc->glDisable(GL_BLEND);
	drawGrids();
	extfunc->glFlush();
	redraw_required = false;
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
	
	save_pixmap_req = false;
	filename_screen_pixmap.clear();
	
#if defined(_USE_GLAPI_QT5_4)   
	uVramTextureID = new QOpenGLTexture(QOpenGLTexture::Target2D);
#else
	uVramTextureID = 0;
#endif
	imgptr = NULL;
	p_emu = NULL;
	extfunc = NULL;
	redraw_required = true;
#ifdef ONE_BOARD_MICRO_COMPUTER
# if defined(_USE_GLAPI_QT5_4)   
	uBitmapTextureID = new QOpenGLTexture(QOpenGLTexture::Target2D);
# else
	uBitmapTextureID = 0;
# endif
	bitmap_uploaded = false;
#endif
#ifdef MAX_BUTTONS
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
	this->initKeyCode();
}

GLDrawClass::~GLDrawClass()
{
//	this->releaseKeyCode();
	emit sig_finished();
#if defined(_USE_GLAPI_QT5_4)   
	if(uVramTextureID->isCreated()) {
  		uVramTextureID->destroy();
	}
	delete uVramTextureID;
#else
	this->deleteTexture(uVramTextureID);
#endif     
#ifdef ONE_BOARD_MICRO_COMPUTER
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
#ifdef MAX_BUTTONS
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
	if(buffer_screen_vertex->isCreated()) buffer_screen_vertex->destroy();
	if(vertex_screen->isCreated()) vertex_screen->destroy();
	if(vertex_grid_vertical->isCreated()) vertex_grid_vertical->destroy();
	if(vertex_grid_horizonal->isCreated()) vertex_grid_horizonal->destroy();
# if defined(ONE_BOARD_MICRO_COMPUTER)
	if(vertex_bitmap->isCreated()) vertex_bitmap->destroy();
# endif
# if defined(MAX_BUTTONS)
	for(i = 0; i < MAX_BUTTONS; i++) {
		if(vertex_button[i]->isCreated()) vertex_button[i]->destroy();
	}
# endif	
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

