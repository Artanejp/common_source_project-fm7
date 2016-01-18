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
		extfunc->glDisable(GL_DEPTH_TEST);
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
#endif	   
		int vertex_loc = prg->attributeLocation("vertex");
		extfunc->glEnableVertexAttribArray(vertex_loc);
		extfunc->glEnableClientState(GL_VERTEX_ARRAY);
   		extfunc->glDrawArrays(GL_LINES, 0, (number + 1) * 2);
		extfunc->glDisableClientState(GL_VERTEX_ARRAY);
		extfunc->glBindBuffer(GL_ARRAY_BUFFER, 0);
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
				  0.15f,
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

void GLDrawClass::drawMain(QOpenGLShaderProgram *prg,
						   QOpenGLVertexArrayObject *vp,
						   QOpenGLBuffer *bp,
						   GLuint texid,
						   QVector4D color,
						   bool f_smoosing)
						   
{
	if(texid != 0) {
		extfunc->glEnable(GL_TEXTURE_2D);
		vp->bind();
		bp->bind();
		prg->bind();
		extfunc->glViewport(0, 0, this->width(), this->height());
		extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
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
		prg->setUniformValue("tex_width",  (float)screen_texture_width); 
		prg->setUniformValue("tex_height", (float)screen_texture_height);
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
		int vertex_loc = prg->attributeLocation("vertex");
		int texcoord_loc = prg->attributeLocation("texcoord");
		extfunc->glEnableVertexAttribArray(vertex_loc);
		extfunc->glEnableVertexAttribArray(texcoord_loc);
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
	{
		main_shader->setUniformValue("color", color);
		drawMain(main_shader, vertex_screen,
				 buffer_screen_vertex, uTmpTextureID,
				 color, smoosing);
	}		
#ifdef ONE_BOARD_MICRO_COMPUTER
	extfunc->glDisable(GL_BLEND);
#endif	
}

void GLDrawClass::uploadMainTexture(QImage *p, bool use_chromakey)
{
	// set vertex
	{
		// Upload to main texture
		extfunc->glBindTexture(GL_TEXTURE_2D, uVramTextureID);
		extfunc->glTexSubImage2D(GL_TEXTURE_2D, 0,
							 0, 0,
							 p->width(), p->height(),
							 GL_BGRA, GL_UNSIGNED_BYTE, p->constBits());
		extfunc->glBindTexture(GL_TEXTURE_2D, 0);
	}
	{
		// Render to tmp-frame buffer and transfer to texture.
		extfunc->glBindFramebuffer(GL_FRAMEBUFFER, uTmpFrameBuffer);
		extfunc->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uTmpTextureID, 0);
		extfunc->glBindRenderbuffer(GL_RENDERBUFFER, uTmpDepthBuffer);
		extfunc->glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, uTmpDepthBuffer);
		
		extfunc->glClearColor(0.0, 0.0, 0.0, 1.0);
		extfunc->glClearDepth(1.0f);
		extfunc->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		QVector4D c(fBrightR, fBrightG, fBrightB, 1.0);
		QVector3D chromakey(0.0, 0.0, 0.0);
		
		tmp_shader->setUniformValue("color", c);
		{
			if(uVramTextureID != 0) {
				extfunc->glEnable(GL_TEXTURE_2D);
				vertex_tmp_texture->bind();
				buffer_vertex_tmp_texture->bind();
				tmp_shader->bind();
				extfunc->glViewport(0, 0, screen_texture_width, screen_texture_height);
				extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
				extfunc->glActiveTexture(GL_TEXTURE0);
				extfunc->glBindTexture(GL_TEXTURE_2D, uVramTextureID);
				extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				if(use_chromakey) {
					tmp_shader->setUniformValue("chromakey", chromakey);
					tmp_shader->setUniformValue("do_chromakey", GL_TRUE);
				} else {
					tmp_shader->setUniformValue("do_chromakey", GL_FALSE);
				}
				//extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
				tmp_shader->setUniformValue("a_texture", 0);
				tmp_shader->setUniformValue("color", c);
				tmp_shader->setUniformValue("tex_width",  (float)screen_texture_width); 
				tmp_shader->setUniformValue("tex_height", (float)screen_texture_height);
				tmp_shader->enableAttributeArray("texcoord");
				tmp_shader->enableAttributeArray("vertex");
				int vertex_loc = tmp_shader->attributeLocation("vertex");
				int texcoord_loc = tmp_shader->attributeLocation("texcoord");
				extfunc->glEnableVertexAttribArray(vertex_loc);
				extfunc->glEnableVertexAttribArray(texcoord_loc);
				extfunc->glEnable(GL_VERTEX_ARRAY);

				extfunc->glDrawArrays(GL_POLYGON, 0, 4);
				
				extfunc->glViewport(0, 0, this->width(), this->height());
				extfunc->glOrtho(0.0f, (float)this->width(), 0.0f, (float)this->height(), -1.0, 1.0);
				buffer_vertex_tmp_texture->release();
				vertex_tmp_texture->release();
		
				tmp_shader->release();
				extfunc->glBindTexture(GL_TEXTURE_2D, 0);
				extfunc->glDisable(GL_TEXTURE_2D);
			}
		}
		extfunc->glBindFramebuffer(GL_FRAMEBUFFER, 0);
		extfunc->glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}
}

void GLDrawClass::drawUpdateTexture(bitmap_t *p)
{
	//p_emu->lock_vm();
	if((p != NULL)) {
#if defined(_USE_GLAPI_QT5_4)   
		if(uVramTextureID->isCreated()) {
			uVramTextureID->destroy();
			uVramTextureID->create();
		}
		uVramTextureID->setData(*p);
#else
		if(uVramTextureID == 0) {
			uVramTextureID = this->bindTexture(p->pImage);
		}
// Will fix at implemenitin PX7.
# if defined(ONE_BOARD_MICRO_COMPUTER) || defined(MAX_BUTTONS)
       		uploadMainTexture(&(p->pImage), true);	
# else
       		uploadMainTexture(&(p->pImage), false);	
# endif
#endif
	}
	//p_emu->unlock_vm();
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
	QFont font = QFont(QString::fromUtf8("Sans"));
	if(button_updated) return;
	col.setRgb(0, 0, 0, 255);
	pen = new QPen(col);
	for(i = 0; i < MAX_BUTTONS; i++) {
		img = new QImage(buttons[i].width * 4, buttons[i].height * 4, QImage::Format_RGB32);
		painter = new QPainter(img);
		painter->setRenderHint(QPainter::Antialiasing, true);
		col.setRgb(255, 255, 255, 255);
		if(strlen(buttons[i].caption) <= 3) {
			font.setPixelSize((buttons[i].width * 4) / 2); 
		} else {
			font.setPixelSize((buttons[i].width * 4) / 4); 
		}
		painter->fillRect(0, 0, buttons[i].width * 4, buttons[i].height * 4, col);
		painter->setFont(font);
		//painter->setPen(pen);
		rect.setWidth(buttons[i].width * 4);
		rect.setHeight(buttons[i].height * 4);
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
	extfunc->glViewport(0, 0, width, height);
	extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
	crt_flag = true;
#if !defined(ONE_BOARD_MICRO_COMPUTER) && !defined(MAX_BUTTONS)
	doSetGridsHorizonal(vert_lines, true);
# if defined(USE_VERTICAL_PIXEL_LINES)		
	doSetGridsVertical(horiz_pixels, true);
# endif		
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
#if !defined(BITMAP_OFFSET_X)
	#define BITMAP_OFFSET_X 0
#endif	   
#if !defined(BITMAP_OFFSET_Y)
	#define BITMAP_OFFSET_Y 0
#endif	   
		vertexBitmap[0].x = -1.0f;
		vertexBitmap[0].y = -1.0f;
	   
		vertexBitmap[1].x = 1.0f - (float)BITMAP_OFFSET_X / (float)SCREEN_WIDTH;
		vertexBitmap[1].y = -1.0f;
	   
		vertexBitmap[2].x = 1.0f - (float)BITMAP_OFFSET_X / (float)SCREEN_WIDTH;
		vertexBitmap[2].y = 1.0f - (float)BITMAP_OFFSET_Y * 2.0 / (float)SCREEN_HEIGHT;
	   
		vertexBitmap[3].x = -1.0f;
		vertexBitmap[3].y = 1.0f - (float)BITMAP_OFFSET_Y * 2.0 / (float)SCREEN_HEIGHT;
	   
		setNormalVAO(bitmap_shader, vertex_bitmap,
					 buffer_bitmap_vertex,
					 vertexBitmap, 4);
	}
#endif
#if defined(MAX_BUTTONS)
	updateButtonTexture();
#endif
	redraw_required = true;
	//do_set_texture_size(imgptr, screen_texture_width, screen_texture_height);
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
		crt_flag = false;
	}
	extfunc->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
#if !defined(ONE_BOARD_MICRO_COMPUTER) && !defined(MAX_BUTTONS)
	drawGrids();
#endif	
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
	uTmpTextureID = 0;
	uTmpFrameBuffer = 0;
	uTmpDepthBuffer = 0;
#endif
	imgptr = NULL;
	screen_texture_width = SCREEN_WIDTH;
	screen_texture_width_old = SCREEN_WIDTH;
	screen_texture_height = SCREEN_HEIGHT;
	screen_texture_height_old = SCREEN_HEIGHT;
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
	screen_multiply = 1.0;
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
	this->deleteTexture(uTmpTextureID);
	extfunc->glDeleteFramebuffers(1, &uTmpFrameBuffer);
	extfunc->glDeleteRenderbuffers(1, &uTmpDepthBuffer);
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

	if(buffer_vertex_tmp_texture->isCreated()) buffer_vertex_tmp_texture->destroy();
	if(vertex_tmp_texture->isCreated()) vertex_tmp_texture->destroy();

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

