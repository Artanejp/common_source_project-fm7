/*
 * qt_glutil_gl2_0.cpp
 * (c) 2016 K.Ohta <whatisthis.sowhat@gmail.com>
 * License: GPLv2.
 * Renderer with OpenGL v2.0 .
 * History:
 * Jan 21, 2016 : Initial.
 */

#include "emu.h"

#include "qt_gldraw.h"
#include "agar_logger.h"
#include "qt_glutil_gl2_0.h"

GLDraw_2_0::GLDraw_2_0(GLDrawClass *parent, EMU *emu) : QObject(parent)
{
	p_wid = parent;
	p_emu = emu;
	
	gl_grid_horiz = false;
	gl_grid_vert = false;
	glVertGrids = NULL;
	glHorizGrids = NULL;

	vert_lines = SCREEN_HEIGHT;
	horiz_pixels = SCREEN_WIDTH;
	set_brightness = false;
	crt_flag = false;
	smoosing = false;
	uVramTextureID = 0;

	imgptr = NULL;
	screen_multiply = 1.0f;
	screen_texture_width = SCREEN_WIDTH;
	screen_texture_width_old = SCREEN_WIDTH;
	screen_texture_height = SCREEN_HEIGHT;
	screen_texture_height_old = SCREEN_HEIGHT;
	p_emu = emu;
	extfunc = NULL;
	redraw_required = false;
#ifdef ONE_BOARD_MICRO_COMPUTER
	uBitmapTextureID = 0;
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
		fButtonX[i] = -1.0 + (float)(vm_buttons[i].x * 2) / (float)SCREEN_WIDTH;
		fButtonY[i] = 1.0 - (float)(vm_buttons[i].y * 2) / (float)SCREEN_HEIGHT;
		fButtonWidth[i] = (float)(vm_buttons[i].width * 2) / (float)SCREEN_WIDTH;
		fButtonHeight[i] = (float)(vm_buttons[i].height * 2) / (float)SCREEN_HEIGHT;
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

	vert_lines = SCREEN_HEIGHT;
	horiz_pixels = SCREEN_WIDTH;
	screen_width = 1.0;
	screen_height = 1.0;
}

GLDraw_2_0::~GLDraw_2_0()
{
	if(buffer_screen_vertex->isCreated()) buffer_screen_vertex->destroy();
	if(vertex_screen->isCreated()) vertex_screen->destroy();
	if(glVertGrids != NULL) free(glVertGrids);
	if(glHorizGrids != NULL) free(glHorizGrids);
#ifdef ONE_BOARD_MICRO_COMPUTER
	if(uBitmapTextureID != 0) {
  		p_wid->deleteTexture(uBitmapTextureID);
	}
#endif
#ifdef MAX_BUTTONS
	int i;
	for(i = 0; i < MAX_BUTTONS; i++) {
		if(uButtonTextureID[i] != 0) p_wid->deleteTexture(uButtonTextureID[i]);
	}
#endif

# if defined(ONE_BOARD_MICRO_COMPUTER)
	if(vertex_bitmap->isCreated()) vertex_bitmap->destroy();
# endif
# if defined(MAX_BUTTONS)
	for(i = 0; i < MAX_BUTTONS; i++) {
		if(vertex_button[i]->isCreated()) vertex_button[i]->destroy();
	}
# endif	
	if(extfunc != NULL) delete extfunc;
}

void GLDraw_2_0::initializeGL(void)
{
}

void GLDraw_2_0::setNormalVAO(QOpenGLShaderProgram *prg,
							   QOpenGLVertexArrayObject *vp,
							   QOpenGLBuffer *bp,
							   VertexTexCoord_t *tp,
							   int size)
{
	int vertex_loc = prg->attributeLocation("vertex");
	int texcoord_loc = prg->attributeLocation("texcoord");

	if(bp == NULL) return;
	if(prg == NULL) return;
	if(!bp->isCreated()) return;

	bp->bind();
	bp->write(0, tp, sizeof(VertexTexCoord_t) * size);
	prg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
	prg->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(VertexTexCoord_t));
	bp->release();

	prg->setUniformValue("a_texture", 0);
			   
}

void GLDraw_2_0::setChangeBrightness(bool flag)
{
	set_brightness = flag;
}

void GLDraw_2_0::setBrightness(GLfloat r, GLfloat g, GLfloat b)
{
	fBrightR = r;
	fBrightG = g;
	fBrightB = b;
}

void GLDraw_2_0::setImgPtr(QImage *p)
{
	imgptr = p;
}

void GLDraw_2_0::setSmoosing(bool flag)
{
	smoosing = flag;
	crt_flag = true;
}

void GLDraw_2_0::setVirtualVramSize(int width, int height)
{
	vert_lines = height;
	horiz_pixels = width;
	crt_flag = true;
}

void GLDraw_2_0::setEmuPtr(EMU *p)
{
	p_emu = p;
}

void GLDraw_2_0::setDrawGLGridVert(bool flag)
{
	gl_grid_vert = flag;
	crt_flag = true;
}

void GLDraw_2_0::setDrawGLGridHoriz(bool flag)
{
	gl_grid_vert = flag;
	crt_flag = true;
}

void GLDraw_2_0::initGLObjects()
{
	extfunc = new QOpenGLFunctions_2_0;
	extfunc->initializeOpenGLFunctions();
	extfunc->glViewport(0, 0, p_wid->width(), p_wid->height());
	extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
}	

void GLDraw_2_0::initFBO(void)
{

# if defined(ONE_BOARD_MICRO_COMPUTER)
   	bitmap_shader = new QOpenGLShaderProgram(p_wid);
	if(bitmap_shader != NULL) {
		bitmap_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
		bitmap_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/normal_fragment_shader.glsl");
		bitmap_shader->link();
	}
# endif
# if defined(MAX_BUTTONS)
	button_shader = new QOpenGLShaderProgram(p_wid);
	if(button_shader != NULL) {
		button_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
		button_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/normal_fragment_shader.glsl");
		button_shader->link();
	}
# endif
	
	glHorizGrids = (GLfloat *)malloc(sizeof(float) * (SCREEN_HEIGHT + 2) * 6);
	if(glHorizGrids != NULL) {
		doSetGridsHorizonal(SCREEN_HEIGHT, true);
	}
	glVertGrids  = (GLfloat *)malloc(sizeof(float) * (SCREEN_WIDTH + 2) * 6);
	if(glVertGrids != NULL) {
		doSetGridsVertical(SCREEN_WIDTH, true);
	}
# if defined(MAX_BUTTONS)
		{
			vertexButtons = new QVector<VertexTexCoord_t>;
			int i;
			for(i = 0; i < MAX_BUTTONS; i++) {
				buffer_button_vertex[i] = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
				buffer_button_vertex[i]->create();
				fButtonX[i] = -1.0 + (float)(vm_buttons[i].x * 2) / (float)SCREEN_WIDTH;
				fButtonY[i] = 1.0 - (float)(vm_buttons[i].y * 2) / (float)SCREEN_HEIGHT;
				fButtonWidth[i] = (float)(vm_buttons[i].width * 2) / (float)SCREEN_WIDTH;
				fButtonHeight[i] = (float)(vm_buttons[i].height * 2) / (float)SCREEN_HEIGHT;
			   
				vertex_button[i] = new QOpenGLVertexArrayObject;
				if(vertex_button[i] != NULL) {
					if(vertex_button[i]->create()) {
						VertexTexCoord_t vt[4];

						vt[0].x =  fButtonX[i];
						vt[0].y =  fButtonY[i];
						vt[0].z =  -0.5f;
						vt[0].s = 0.0f;
						vt[0].t = 1.0f;
						
						vt[1].x =  fButtonX[i] + fButtonWidth[i];
						vt[1].y =  fButtonY[i];
						vt[1].z =  -0.5f;
						vt[1].s = 1.0f;
						vt[1].t = 1.0f;
						
						vt[2].x =  fButtonX[i] + fButtonWidth[i];
						vt[2].y =  fButtonY[i] - fButtonHeight[i];
						vt[2].z =  -0.5f;
						vt[2].s = 1.0f;
						vt[2].t = 0.0f;
						
						vt[3].x =  fButtonX[i];
						vt[3].y =  fButtonY[i] - fButtonHeight[i];
						vt[3].z =  -0.5f;
						vt[3].s = 0.0f;
						vt[3].t = 0.0f;
				
						vertexButtons->append(vt[0]);
						vertexButtons->append(vt[1]);
						vertexButtons->append(vt[2]);
						vertexButtons->append(vt[3]);
						vertex_button[i]->bind();
						buffer_button_vertex[i]->bind();
						buffer_button_vertex[i]->allocate(4 * sizeof(VertexTexCoord_t));
						
						buffer_button_vertex[i]->setUsagePattern(QOpenGLBuffer::StaticDraw);
						buffer_button_vertex[i]->release();
						vertex_button[i]->release();
						setNormalVAO(button_shader, vertex_button[i],
									 buffer_button_vertex[i],
									 vt, 4);
					}
				}
			}
		}
#endif
#if defined(ONE_BOARD_MICRO_COMPUTER)
	   buffer_bitmap_vertex = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	   vertex_bitmap = new QOpenGLVertexArrayObject;
	   vertexBitmap[0].x = -1.0f;
	   vertexBitmap[0].y = -1.0f;
	   vertexBitmap[0].z = -0.1f;
	   vertexBitmap[0].s = 0.0f;
	   vertexBitmap[0].t = 0.0f;
	   
	   vertexBitmap[1].x = +1.0f;
	   vertexBitmap[1].y = -1.0f;
	   vertexBitmap[1].z = -0.1f;
	   vertexBitmap[1].s = 1.0f;
	   vertexBitmap[1].t = 0.0f;
	   
	   vertexBitmap[2].x = +1.0f;
	   vertexBitmap[2].y = +1.0f;
	   vertexBitmap[2].z = -0.1f;
	   vertexBitmap[2].s = 1.0f;
	   vertexBitmap[2].t = 1.0f;
	   
	   vertexBitmap[3].x = -1.0f;
	   vertexBitmap[3].y = +1.0f;
	   vertexBitmap[3].z = -0.1f;
	   vertexBitmap[3].s = 0.0f;
	   vertexBitmap[3].t = 1.0f;
	   
	   if(vertex_bitmap != NULL) {
		   if(vertex_bitmap->create()) {
			   buffer_bitmap_vertex->create();
			   buffer_bitmap_vertex->setUsagePattern(QOpenGLBuffer::StaticDraw);
			   
			   {
				   QVector4D c;
				   c = QVector4D(1.0, 1.0, 1.0, 1.0);
				   bitmap_shader->setUniformValue("color", c);
			   }
			   vertex_bitmap->bind();
			   buffer_bitmap_vertex->bind();
			   buffer_bitmap_vertex->allocate(sizeof(vertexBitmap));
			   buffer_bitmap_vertex->release();
			   vertex_bitmap->release();
			   setNormalVAO(bitmap_shader, vertex_bitmap,
							buffer_bitmap_vertex,
							vertexBitmap, 4);
		   }
	   }
#endif
	// Init view
	extfunc->glClearColor(0.0, 0.0, 0.0, 1.0);
}

void GLDraw_2_0::initLocalGLObjects(void)
{
	main_shader = new QOpenGLShaderProgram(p_wid);
	if(main_shader != NULL) {
		main_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
		main_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/chromakey_fragment_shader.glsl");
		main_shader->link();
	}
	buffer_screen_vertex = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	vertex_screen = new QOpenGLVertexArrayObject;
	
	vertexFormat[0].x = -0.5f;
	vertexFormat[0].y = -0.5f;
	vertexFormat[0].z = -0.9f;
	vertexFormat[0].s = 0.0f;
	vertexFormat[0].t = 1.0f;
			
	vertexFormat[1].x = +0.5f;
	vertexFormat[1].y = -0.5f;
	vertexFormat[1].z = -0.9f;
	vertexFormat[1].s = 1.0f;
	vertexFormat[1].t = 1.0f;
			
	vertexFormat[2].x = +0.5f;
	vertexFormat[2].y = +0.5f;
	vertexFormat[2].z = -0.9f;
	vertexFormat[2].s = 1.0f;
	vertexFormat[2].t = 0.0f;
	
	vertexFormat[3].x = -0.5f;
	vertexFormat[3].y = +0.5f;
	vertexFormat[3].z = -0.9f;
	vertexFormat[3].s = 0.0f;
	vertexFormat[3].t = 0.0f;
	if(buffer_screen_vertex != NULL) {
		if(buffer_screen_vertex->create()) {
			{
				QVector4D c;
				c = QVector4D(1.0, 1.0, 1.0, 1.0);
				main_shader->setUniformValue("color", c);
			}
			
			buffer_screen_vertex->create();
			buffer_screen_vertex->setUsagePattern(QOpenGLBuffer::DynamicDraw);
			
			if(vertex_screen != NULL) vertex_screen->bind();
			buffer_screen_vertex->bind();
			buffer_screen_vertex->allocate(sizeof(VertexTexCoord_t) * 4);
			if(vertex_screen != NULL) vertex_screen->release();
			buffer_screen_vertex->release();
			setNormalVAO(main_shader, vertex_screen,
						 buffer_screen_vertex,
						 vertexFormat, 4);
		}
	}
}

void GLDraw_2_0::doSetGridsHorizonal(int lines, bool force)
{
	int i;
	GLfloat yf;
	GLfloat delta;
	
	if((lines == vert_lines) && !force) return;
	vert_lines = lines;
	yf = -screen_height;
	if(vert_lines <= 0) return;
	if(vert_lines > SCREEN_HEIGHT) vert_lines = SCREEN_HEIGHT;
	
	delta = (2.0f * screen_height) / (float)vert_lines;
	yf = yf - delta * 1.0f;
	if(glHorizGrids != NULL) {
		for(i = 0; i < (vert_lines + 1) ; i++) {
			glHorizGrids[i * 6]     = -screen_width; // XBegin
			glHorizGrids[i * 6 + 3] = +screen_width; // XEnd
			glHorizGrids[i * 6 + 1] = yf; // YBegin
			glHorizGrids[i * 6 + 4] = yf; // YEnd
			glHorizGrids[i * 6 + 2] = -0.95f; // ZBegin
			glHorizGrids[i * 6 + 5] = -0.95f; // ZEnd
			yf = yf + delta;
		}
	}
}

void GLDraw_2_0::doSetGridsVertical(int pixels, bool force)
{
	int i;
	GLfloat xf;
	GLfloat delta;
	
	if((pixels == horiz_pixels) && !force) return;
	horiz_pixels = pixels;
	if(horiz_pixels <= 0) return;
	if(horiz_pixels > SCREEN_WIDTH) horiz_pixels = SCREEN_WIDTH;
	
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
			glVertGrids[i * 6 + 2] = -0.95f; // ZBegin
			glVertGrids[i * 6 + 5] = -0.95f; // ZEnd
			xf = xf + delta;
		}
	}
}
void GLDraw_2_0::drawGridsMain(GLfloat *tp,
							   int number,
							   GLfloat lineWidth,
							   QVector4D color)
{
	if(number <= 0) return;
	extfunc->glDisable(GL_TEXTURE_2D);
	extfunc->glDisable(GL_DEPTH_TEST);
	extfunc->glDisable(GL_BLEND);
	{ 
		if(tp != NULL) {
			int i;
			int p = 0;
			extfunc->glColor4f(color.x(), color.y(), color.z(), color.w());
			extfunc->glLineWidth(lineWidth);
			extfunc->glBegin(GL_LINES);
			for(i = 0; i < (number + 1); i++) {
				extfunc->glVertex3f(tp[p + 0], tp[p + 1], tp[p + 2]);
				extfunc->glVertex3f(tp[p + 3], tp[p + 4], tp[p + 5]);
				p += 6;
			}
			extfunc->glEnd();
		}
	}
}

void GLDraw_2_0::drawGridsHorizonal(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	drawGridsMain(glHorizGrids,
				  vert_lines,
				  0.15f,
				  c);
}

void GLDraw_2_0::drawGridsVertical(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	drawGridsMain(glVertGrids,
				  horiz_pixels,
				  0.5f,
				  c);
}


void GLDraw_2_0::drawGrids(void)
{
	gl_grid_horiz = config.opengl_scanline_horiz;
	gl_grid_vert  = config.opengl_scanline_vert;
	if(gl_grid_horiz && (vert_lines > 0)) {
		drawGridsHorizonal();
	}
#if defined(USE_VERTICAL_PIXEL_LINES)		
	if(gl_grid_vert && (horiz_pixels > 0)) {
		drawGridsVertical();
	}
#endif	
}

#if defined(MAX_BUTTONS)
void GLDraw_2_0::updateButtonTexture(void)
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
		img = new QImage(vm_buttons[i].width * 4, vm_buttons[i].height * 4, QImage::Format_RGB32);
		painter = new QPainter(img);
		painter->setRenderHint(QPainter::Antialiasing, true);
		col.setRgb(255, 255, 255, 255);
		if(strlen(vm_buttons[i].caption) <= 3) {
			font.setPixelSize((vm_buttons[i].width * 4) / 2); 
		} else {
			font.setPixelSize((vm_buttons[i].width * 4) / 4); 
		}
		painter->fillRect(0, 0, vm_buttons[i].width * 4, vm_buttons[i].height * 4, col);
		painter->setFont(font);
		//painter->setPen(pen);
		rect.setWidth(vm_buttons[i].width * 4);
		rect.setHeight(vm_buttons[i].height * 4);
		rect.setX(0);
		rect.setY(0);
		painter->drawText(rect, Qt::AlignCenter, QString::fromUtf8(vm_buttons[i].caption));
		if(uButtonTextureID[i] != 0) {
	  		p_wid->deleteTexture(uButtonTextureID[i]);
		}
		uButtonTextureID[i] = p_wid->bindTexture(*img);;
		delete painter;
		delete img;
	}
	delete pen;
	button_updated = true;
}

void GLDraw_2_0::drawButtons()
{
	int i;
	//updateButtonTexture();
	for(i = 0; i < MAX_BUTTONS; i++) {
		QVector4D c;
		VertexTexCoord_t vt[4];
		vt[0].x =  fButtonX[i];
		vt[0].y =  fButtonY[i];
		vt[0].z =  -0.5f;
		vt[0].s = 0.0f;
		vt[0].t = 1.0f;
		
		vt[1].x =  fButtonX[i] + fButtonWidth[i];
		vt[1].y =  fButtonY[i];
		vt[1].z =  -0.5f;
		vt[1].s = 1.0f;
		vt[1].t = 1.0f;
		
		vt[2].x =  fButtonX[i] + fButtonWidth[i];
		vt[2].y =  fButtonY[i] - fButtonHeight[i];
		vt[2].z =  -0.5f;
		vt[2].s = 1.0f;
		vt[2].t = 0.0f;
		
		vt[3].x =  fButtonX[i];
		vt[3].y =  fButtonY[i] - fButtonHeight[i];
		vt[3].z =  -0.5f;
		vt[3].s = 0.0f;
		vt[3].t = 0.0f;
		c = QVector4D(1.0, 1.0, 1.0, 1.0);
		drawMain(button_shader, vertex_button[i],
				 buffer_button_vertex[i],
				 vt,
				 uButtonTextureID[i],
				 c, false);
	}
}
#endif


# ifdef ONE_BOARD_MICRO_COMPUTER
void GLDraw_2_0::drawBitmapTexture(void)
{
	QVector4D c;
	c = QVector4D(1.0, 1.0, 1.0, 1.0);
	drawMain(bitmap_shader, vertex_bitmap,
			 buffer_bitmap_vertex,
			 vertexBitmap,
			 uBitmapTextureID,
			 c, false);
}
# endif

void GLDraw_2_0::drawScreenTexture(void)
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
#ifdef ONE_BOARD_MICRO_COMPUTER
	{
		QVector3D cc = QVector3D(0.0, 0.0, 0.0);
		drawMain(main_shader, vertex_screen,
				 buffer_screen_vertex,
				 vertexFormat,
				 uVramTextureID, // v2.0
				 color, smoosing,
				 true, cc);
		extfunc->glDisable(GL_BLEND);
	}
#else
	{
		drawMain(main_shader, vertex_screen,
				 buffer_screen_vertex,
				 vertexFormat,
				 uVramTextureID, // v2.0
				 color, smoosing);
	}		
#endif	
}

void GLDraw_2_0::drawMain(QOpenGLShaderProgram *prg,
						  QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  VertexTexCoord_t *vertex_data,
						  GLuint texid,
						  QVector4D color,
						  bool f_smoosing,
						  bool do_chromakey,
						  QVector3D chromakey)
						   
{
	if(texid != 0) {
		extfunc->glEnable(GL_TEXTURE_2D);

		extfunc->glViewport(0, 0, p_wid->width(), p_wid->height());
		extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
		extfunc->glBindTexture(GL_TEXTURE_2D, texid);

		if(!f_smoosing) {
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
		} else {
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		}

		if((bp != NULL) && (vp != NULL) && (prg != NULL)) {
			if((bp->isCreated()) && (vp->isCreated()) && (prg->isLinked())) {
				bp->bind();
				vp->bind();
				prg->bind();
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
				if(do_chromakey) {
					prg->setUniformValue("chromakey", chromakey);
					prg->setUniformValue("do_chromakey", GL_TRUE);
				} else {
					prg->setUniformValue("do_chromakey", GL_FALSE);
				}			
				int vertex_loc = prg->attributeLocation("vertex");
				int texcoord_loc = prg->attributeLocation("texcoord");
				prg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
				prg->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(VertexTexCoord_t));
				prg->enableAttributeArray(vertex_loc);
				prg->enableAttributeArray(texcoord_loc);
				
				extfunc->glEnableClientState(GL_VERTEX_ARRAY);
				extfunc->glEnableClientState(GL_TEXTURE_COORD_ARRAY);
				extfunc->glColor3f(1.0f, 1.0f, 1.0f);
				
				extfunc->glVertexPointer(3, GL_FLOAT, sizeof(VertexTexCoord_t), (void *)0);
				extfunc->glTexCoordPointer(2, GL_FLOAT, sizeof(VertexTexCoord_t), (void *)(0 + 3 * sizeof(GLfloat)));
				extfunc->glDrawArrays(GL_POLYGON, 0, 4);
				extfunc->glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				extfunc->glDisableClientState(GL_VERTEX_ARRAY);
				
				bp->release();
				vp->release();
				prg->release();
				extfunc->glBindTexture(GL_TEXTURE_2D, 0);
				extfunc->glDisable(GL_TEXTURE_2D);
				return;
			}
		}
		
		{ // Fallback
			int i;
			extfunc->glBegin(GL_POLYGON);
			for(i = 0; i < 4; i++) {
				extfunc->glTexCoord2f(vertex_data[i].s, vertex_data[i].t);
				extfunc->glVertex3f(vertex_data[i].x, vertex_data[i].y, vertex_data[i].z);
			}
			extfunc->glEnd();
			extfunc->glBindTexture(GL_TEXTURE_2D, 0);
			extfunc->glDisable(GL_TEXTURE_2D);
		}
	}
}


#if defined(ONE_BOARD_MICRO_COMPUTER)
void GLDraw_2_0::uploadBitmapTexture(QImage *p)
{
	int i;
	if(p == NULL) return;
	if(!bitmap_uploaded) {
		if(uBitmapTextureID != 0) {
	  		p_wid->deleteTexture(uBitmapTextureID);
		}
		uBitmapTextureID = p_wid->bindTexture(*p);
		bitmap_uploaded = true;
		crt_flag = true;
	}
}

void GLDraw_2_0::updateBitmap(QImage *p)
{
	redraw_required = true;
	bitmap_uploaded = false;
	uploadBitmapTexture(p);
}
#endif

void GLDraw_2_0::uploadMainTexture(QImage *p, bool use_chromakey)
{
	// set vertex
	redraw_required = true;
	imgptr = p;
	if(p == NULL) return;
	{
		if(use_chromakey) {
			bool checkf = false;
			if(vertex_screen == NULL) {
				checkf = true;
			} else {
				if(!vertex_screen->isCreated()) checkf = true;
			}
			if(buffer_screen_vertex == NULL) {
				checkf = true;
			} else {
				if(!buffer_screen_vertex->isCreated()) checkf = true;
			}
			if(main_shader == NULL) {
				checkf = true;
			} else {
				if(!main_shader->isLinked()) checkf = true;
			}
			if(checkf) {
				QColor mask = QColor(0, 0, 0, 255);
				QImage pp = p->QImage::createMaskFromColor(mask.rgba(), Qt::MaskOutColor);
				if(uVramTextureID == 0) {
					uVramTextureID = p_wid->bindTexture(pp);
				}
				extfunc->glBindTexture(GL_TEXTURE_2D, uVramTextureID);
				extfunc->glTexSubImage2D(GL_TEXTURE_2D, 0,
										 0, 0,
										 pp.width(), pp.height(),
										 GL_BGRA, GL_UNSIGNED_BYTE, pp.constBits());
				extfunc->glBindTexture(GL_TEXTURE_2D, 0);
				crt_flag = true;
				return;
			}
		}
		// Upload to main texture
		if(uVramTextureID == 0) {
			uVramTextureID = p_wid->bindTexture(*p);
		}
		extfunc->glBindTexture(GL_TEXTURE_2D, uVramTextureID);
		extfunc->glTexSubImage2D(GL_TEXTURE_2D, 0,
								 0, 0,
								 p->width(), p->height(),
								 GL_BGRA, GL_UNSIGNED_BYTE, p->constBits());
		extfunc->glBindTexture(GL_TEXTURE_2D, 0);
	}
	crt_flag = true;
}

void GLDraw_2_0::resizeGL(int width, int height)
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
	vertexFormat[0].x = -screen_width;
	vertexFormat[0].y = -screen_height;
	
	vertexFormat[1].x = +screen_width;
	vertexFormat[1].y = -screen_height;
	
	vertexFormat[2].x = +screen_width;
	vertexFormat[2].y = +screen_height;
	
	vertexFormat[3].x = -screen_width;
	vertexFormat[3].y = +screen_height;
	
	if(buffer_screen_vertex->isCreated()) {
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
}

void GLDraw_2_0::paintGL(void)
{
	int i;
	if(!crt_flag && !redraw_required) return;
	if(p_emu != NULL) {
		crt_flag = false;
	}
	redraw_required = false;
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
	extfunc->glDisable(GL_BLEND);
#if !defined(ONE_BOARD_MICRO_COMPUTER) && !defined(MAX_BUTTONS)
	drawGrids();
#endif	
	extfunc->glFlush();
}

void GLDraw_2_0::do_set_screen_multiply(float mul)
{
	screen_multiply = mul;
	do_set_texture_size(imgptr, screen_texture_width, screen_texture_height);
}


void GLDraw_2_0::do_set_texture_size(QImage *p, int w, int h)
{
	if(w <= 0) w = SCREEN_WIDTH;
	if(h <= 0) h = SCREEN_HEIGHT;
	float wfactor = 1.0f;
	float hfactor = 1.0f;
	float iw, ih;
	if(p != NULL) {
		int ww = w;
		int hh = h;
		imgptr = p;
		iw = (float)p->width();
		ih = (float)p->height();
		//if(screen_multiply < 1.0f) {
		p_wid->makeCurrent();
		screen_texture_width = w;
		screen_texture_height = h;
		vertexFormat[0].s = 0.0f;
		vertexFormat[0].t = (float)hh / ih;
		vertexFormat[1].s = (float)ww / iw;
		vertexFormat[1].t = (float)hh / ih;
		vertexFormat[2].s = (float)ww / iw;
		vertexFormat[2].t = 0.0f;
		vertexFormat[3].s = 0.0f;
		vertexFormat[3].t = 0.0f;
		
		setNormalVAO(main_shader, vertex_screen,
					 buffer_screen_vertex,
					 vertexFormat, 4);
		p_wid->doneCurrent();
	}		
	this->doSetGridsHorizonal(h, true);
	this->doSetGridsVertical(w, true);
}
