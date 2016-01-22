/*
 * qt_glutil_2_0.cpp
 * (c) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 * Modified to Common Source code Project, License is changed to GPLv2.
 * Specify for OpenGL 2.0
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

	vp->bind();
	bp->bind();

	bp->write(0, tp, sizeof(VertexTexCoord_t) * size);
	prg->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, sizeof(VertexTexCoord_t));
	prg->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, sizeof(VertexTexCoord_t));
	bp->release();
	vp->release();
	prg->setUniformValue("a_texture", 0);
			   
	extfunc->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord_t), 0); 
	extfunc->glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTexCoord_t), 
							       (char *)NULL + 3 * sizeof(GLfloat)); 
	prg->enableAttributeArray(vertex_loc);
	prg->enableAttributeArray(texcoord_loc);
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

	grids_shader_horizonal = new QOpenGLShaderProgram(p_wid);
#if defined(USE_SCREEN_ROTATE)   
	if(grids_shader_horizonal != NULL) {
		grids_shader_horizonal->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grids_vertex_shader.glsl");
		grids_shader_horizonal->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grids_fragment_shader.glsl");
		grids_shader_horizonal->link();
	}
	grids_shader_vertical = new QOpenGLShaderProgram(p_wid);
	if(grids_shader_vertical != NULL) {
		grids_shader_vertical->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grids_vertex_shader.glsl");
		grids_shader_vertical->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grids_fragment_shader.glsl");
		grids_shader_vertical->link();
	}
#else
	if(grids_shader_horizonal != NULL) {
		grids_shader_horizonal->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grids_vertex_shader_fixed.glsl");
		grids_shader_horizonal->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grids_fragment_shader.glsl");
		grids_shader_horizonal->link();
	}
	grids_shader_vertical = new QOpenGLShaderProgram(p_wid);
	if(grids_shader_vertical != NULL) {
		grids_shader_vertical->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grids_vertex_shader_fixed.glsl");
		grids_shader_vertical->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grids_fragment_shader.glsl");
		grids_shader_vertical->link();
	}
#endif
# if defined(ONE_BOARD_MICRO_COMPUTER)
   	bitmap_shader = new QOpenGLShaderProgram(p_wid);
	if(bitmap_shader != NULL) {
		bitmap_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
		bitmap_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/normal_fragment_shader.glsl");
		bitmap_shader->link();
	}
# endif
# if defined(MAX_BUTTONS)
	for(int i = 0; i < MAX_BUTTONS; i++) {
		button_shader[i] = new QOpenGLShaderProgram(p_wid);
		if(button_shader[i] != NULL) {
			button_shader[i]->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
			button_shader[i]->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/normal_fragment_shader.glsl");
			button_shader[i]->link();
		}
	}
# endif
	
	glHorizGrids = (GLfloat *)malloc(sizeof(float) * (SCREEN_HEIGHT + 2) * 6);
	if(glHorizGrids != NULL) {
		buffer_grid_horizonal = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		vertex_grid_horizonal = new QOpenGLVertexArrayObject;
		
		screen_height = 1.0f;
		vert_lines = SCREEN_HEIGHT;
		if(vertex_grid_horizonal != NULL) {
			if(vertex_grid_horizonal->create()) {
				buffer_grid_horizonal->create();
				vertex_grid_horizonal->bind();
				buffer_grid_horizonal->bind();
				buffer_grid_horizonal->allocate((vert_lines + 1) * 6 * sizeof(GLfloat));
				buffer_grid_horizonal->setUsagePattern(QOpenGLBuffer::StaticDraw);
				buffer_grid_horizonal->release();
				vertex_grid_horizonal->release();
		
			}
			doSetGridsHorizonal(SCREEN_HEIGHT, true);
		}
	}
	glVertGrids  = (GLfloat *)malloc(sizeof(float) * (SCREEN_WIDTH + 2) * 6);
	if(glVertGrids != NULL) {
		buffer_grid_vertical = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		vertex_grid_vertical = new QOpenGLVertexArrayObject;
		if(vertex_grid_vertical != NULL) {
			if(vertex_grid_vertical->create()) {
				buffer_grid_vertical->bind();
				vertex_grid_vertical->bind();
				buffer_grid_vertical->allocate((SCREEN_WIDTH + 1) * 6 * sizeof(GLfloat));
				buffer_grid_vertical->setUsagePattern(QOpenGLBuffer::StaticDraw);
				vertex_grid_vertical->release();
				buffer_grid_vertical->release();
				doSetGridsVertical(SCREEN_WIDTH, true);
			}
		}
	}
# if defined(MAX_BUTTONS)
		{
			vertexButtons = new QVector<VertexTexCoord_t>;
			int i;
			for(i = 0; i < MAX_BUTTONS; i++) {
				buffer_button_vertex[i] = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
				buffer_button_vertex[i]->create();
				fButtonX[i] = -1.0 + (float)(buttons[i].x * 2) / (float)SCREEN_WIDTH;
				fButtonY[i] = 1.0 - (float)(buttons[i].y * 2) / (float)SCREEN_HEIGHT;
				fButtonWidth[i] = (float)(buttons[i].width * 2) / (float)SCREEN_WIDTH;
				fButtonHeight[i] = (float)(buttons[i].height * 2) / (float)SCREEN_HEIGHT;
			   
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
						setNormalVAO(button_shader[i], vertex_button[i],
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
	   if(vertex_bitmap != NULL) {
		   if(vertex_bitmap->create()) {
			   {
				   QVector4D c;
				   c = QVector4D(1.0, 1.0, 1.0, 1.0);
				   bitmap_shader->setUniformValue("color", c);
			   }
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
			   
			   buffer_bitmap_vertex->create();
			   buffer_bitmap_vertex->setUsagePattern(QOpenGLBuffer::StaticDraw);
			   int vertex_loc = bitmap_shader->attributeLocation("vertex");
			   int texcoord_loc = bitmap_shader->attributeLocation("texcoord");
			   
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
	
	bGL_PIXEL_UNPACK_BUFFER_BINDING = false;
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
	if(vertex_screen != NULL) {
		if(vertex_screen->create()) {
			{
				QVector4D c;
				c = QVector4D(1.0, 1.0, 1.0, 1.0);
				main_shader->setUniformValue("color", c);
			}
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
			
			
			buffer_screen_vertex->create();
			buffer_screen_vertex->setUsagePattern(QOpenGLBuffer::DynamicDraw);
			
			vertex_screen->bind();
			buffer_screen_vertex->bind();
			buffer_screen_vertex->allocate(sizeof(VertexTexCoord_t) * 4);
			vertex_screen->release();
			buffer_screen_vertex->release();
			setNormalVAO(main_shader, vertex_screen,
						 buffer_screen_vertex,
						 vertexFormat, 4);
			//QMatrix4x4 mat;
			//mat.ortho(-1.0, 1.0, -1.0, +1.0, -1.0, 1.0);
			//mat.translate(0, 0, 0);
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
	//AGAR_DebugLog(AGAR_LOG_DEBUG, "Set H-Lines: %d (%dx%d), %f, %d", vert_lines, p_wid->width(), p_wid->height()
	//			  ,delta, vertex_grid_horizonal->isCreated()? 1 : 0);
	if(vertex_grid_horizonal->isCreated()) {
		vertex_grid_horizonal->bind();
		buffer_grid_horizonal->bind();
		buffer_grid_horizonal->allocate((vert_lines + 1) * 6 * sizeof(GLfloat));
		buffer_grid_horizonal->write(0, glHorizGrids, (vert_lines + 1) * 6 * sizeof(GLfloat));
		
		grids_shader_horizonal->bind();
		int vertex_loc = grids_shader_horizonal->attributeLocation("vertex");
		grids_shader_horizonal->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3);
		extfunc->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		grids_shader_horizonal->release();
		
		buffer_grid_horizonal->release();
		vertex_grid_horizonal->release();
		grids_shader_horizonal->enableAttributeArray(vertex_loc);
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
		if(vertex_grid_vertical->isCreated()) {
			vertex_grid_vertical->bind();
			buffer_grid_vertical->bind();
			buffer_grid_vertical->allocate((horiz_pixels + 1) * 6 * sizeof(GLfloat));
			buffer_grid_vertical->write(0, glVertGrids, (horiz_pixels + 1)* 6 * sizeof(GLfloat));
			
			grids_shader_vertical->bind();
			int vertex_loc = grids_shader_vertical->attributeLocation("vertex");
			grids_shader_vertical->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3);
			extfunc->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
			grids_shader_vertical->release();
			
			buffer_grid_vertical->release();
			vertex_grid_vertical->release();
		}
	}
}

void GLDraw_2_0::drawGridsMain(QOpenGLShaderProgram *prg,
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
		//extfunc->glViewport(0, 0, p_wid->width(), p_wid->height());
		//extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
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

void GLDraw_2_0::drawGridsHorizonal(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	drawGridsMain(grids_shader_horizonal,
				  vertex_grid_horizonal,
				  buffer_grid_horizonal,
				  vert_lines,
				  0.15f,
				  c);
}

void GLDraw_2_0::drawGridsVertical(void)
{
	QVector4D c= QVector4D(0.0f, 0.0f, 0.0f, 1.0f);
	drawGridsMain(grids_shader_vertical,
				  vertex_grid_vertical,
				  buffer_grid_vertical,
				  horiz_pixels,
				  0.5f,
				  c);
}


void GLDraw_2_0::drawGrids(void)
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
		c = QVector4D(1.0, 1.0, 1.0, 1.0);
		drawMain(button_shader[i], vertex_button[i],
				 buffer_button_vertex[i], uButtonTextureID[i],
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
			 buffer_bitmap_vertex, uBitmapTextureID,
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
	{
		drawMain(main_shader, vertex_screen,
				 buffer_screen_vertex, uVramTextureID, // v2.0
				 color, smoosing);
	}		
#ifdef ONE_BOARD_MICRO_COMPUTER
	extfunc->glDisable(GL_BLEND);
#endif	
}

void GLDraw_2_0::drawMain(QOpenGLShaderProgram *prg,
						  QOpenGLVertexArrayObject *vp,
						  QOpenGLBuffer *bp,
						  GLuint texid,
						  QVector4D color,
						  bool f_smoosing,
						  bool do_chromakey,
						  QVector3D chromakey)
						   
{
	if(texid != 0) {
		extfunc->glEnable(GL_TEXTURE_2D);
		vp->bind();
		bp->bind();
		prg->bind();
		extfunc->glViewport(0, 0, p_wid->width(), p_wid->height());
		extfunc->glOrtho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0, 1.0);
		extfunc->glActiveTexture(GL_TEXTURE0);
		extfunc->glBindTexture(GL_TEXTURE_2D, texid);
		if(!f_smoosing) {
			extfunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
		if(do_chromakey) {
			prg->setUniformValue("chromakey", chromakey);
			prg->setUniformValue("do_chromakey", GL_TRUE);
		} else {
			prg->setUniformValue("do_chromakey", GL_FALSE);
		}			
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
	if(uVramTextureID == 0) {
		uVramTextureID = p_wid->bindTexture(*p);
	}
	{
		// Upload to main texture
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
