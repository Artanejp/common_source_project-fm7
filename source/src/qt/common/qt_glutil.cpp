/*
 * Qt: OpenGL Utils
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */

#undef _USE_OPENCL
#ifdef _USE_OPENCL
//#include "agar_glcl.h"
#endif
#include "agar_logger.h"
#include "qt_gldraw.h"
#include "emu.h"
#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP
#include <QGLContext>
#include <QImage>

//extern const char *cl_render;

void GLDrawClass::update_screen(QImage *p)
{
	//if(tick < (1000 / 75)) tick = 1000 / 75;
	imgptr = p;
	crt_flag = true;
	this->update();
}



void GLDrawClass::InitContextCL(void)
{
}



void GLDrawClass::initializeGL(void)
{
	int i;
	GLfloat xf, yf, delta;
#ifdef _USE_OPENCL
	cldraw = NULL;
#endif
	/*
	 * GL 拡張の取得 20110907-
	 */
	InitFBO(); // 拡張の有無を調べてからFBOを初期化する。
	InitGLExtensionVars();
	
	// Init view
	extfunc->glClearColor(0.0, 0.0, 0.0, 1.0);
	
}

void GLDrawClass::setChangeBrightness(bool flag)
{
	set_brightness = flag;
}

void GLDrawClass::setBrightness(GLfloat r, GLfloat g, GLfloat b)
{
	fBrightR = r;
	fBrightG = g;
	fBrightB = b;
}

void GLDrawClass::setSmoosing(bool flag)
{
	smoosing = flag;
	crt_flag = true;
}

void GLDrawClass::setDrawGLGridVert(bool flag)
{
	gl_grid_vert = flag;
	crt_flag = true;
}

void GLDrawClass::setDrawGLGridHoriz(bool flag)
{
	gl_grid_vert = flag;
	crt_flag = true;
}

void GLDrawClass::setVirtualVramSize(int width, int height)
{
	vert_lines = height;
	horiz_pixels = width;
	crt_flag = true;
}

// OpenGL状態変数
bool GLDrawClass::QueryGLExtensions(const char *str)
{
	char *ext;
	char *p;
	int i;
	int j;
	int k;
	int l;
	int ll;
	return false;
}

void GLDrawClass::InitGLExtensionVars(void)
{
	//    bGL_PIXEL_UNPACK_BUFFER_BINDING = QueryGLExtensions("GL_pixel_unpack_buffer_binding");
	bGL_PIXEL_UNPACK_BUFFER_BINDING = true;
	bCLEnabled = false;
	bCLGLInterop = false;
}


void GLDrawClass::setNormalVAO(QOpenGLShaderProgram *prg,
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

void GLDrawClass::doSetGridsHorizonal(int lines, bool force)
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

void GLDrawClass::doSetGridsVertical(int pixels, bool force)
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
			glVertGrids[i * 6 + 2] = -0.1f; // ZBegin
			glVertGrids[i * 6 + 5] = -0.1f; // ZEnd
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
	
void GLDrawClass::InitFBO(void)
{
	int i;
	GLfloat xf, yf, delta;
	
	bGL_EXT_VERTEX_ARRAY = false;
#if defined(_USE_GLAPI_QT5_4) || defined(_USE_GLAPI_QT5_1)
	extfunc = new QOpenGLFunctions;
	extfunc->initializeOpenGLFunctions();
#elif defined(_USE_GLAPI_QT4_8) || defined(_USE_GLAPI_QT5_0)
	extfunc = new QGLFunctions;
	//if(extfunc->initializeOpenGLFunctions()) bGL_EXT_VERTEX_ARRAY = true;
#endif   
	main_shader = new QOpenGLShaderProgram(this);
	if(main_shader != NULL) {
		main_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
#if defined(USE_BITMAP) || defined(USE_BUTTON)
		main_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/chromakey_fragment_shader.glsl");
#else
		main_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragment_shader.glsl");
#endif		
		main_shader->link();
	}
	grids_shader_horizonal = new QOpenGLShaderProgram(this);
	if(grids_shader_horizonal != NULL) {
		grids_shader_horizonal->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grids_vertex_shader.glsl");
		grids_shader_horizonal->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grids_fragment_shader.glsl");
		grids_shader_horizonal->link();
	}
	grids_shader_vertical = new QOpenGLShaderProgram(this);
	if(grids_shader_vertical != NULL) {
		grids_shader_vertical->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grids_vertex_shader.glsl");
		grids_shader_vertical->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grids_fragment_shader.glsl");
		grids_shader_vertical->link();
	}

# if defined(USE_BITMAP)
   	bitmap_shader = new QOpenGLShaderProgram(this);
	if(bitmap_shader != NULL) {
		bitmap_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
		bitmap_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/normal_fragment_shader.glsl");
		bitmap_shader->link();
	}
# endif
# if defined(USE_BUTTON)
	for(i = 0; i < MAX_BUTTONS; i++) {
		button_shader[i] = new QOpenGLShaderProgram(this);
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
# if defined(USE_BUTTON)
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
#if defined(USE_BITMAP)
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
			   int vertex_loc = main_shader->attributeLocation("vertex");
			   int texcoord_loc = main_shader->attributeLocation("texcoord");
			   
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
			   QMatrix4x4 mat;
			   mat.ortho(-1.0, 1.0, -1.0, +1.0, -1.0, 1.0);
			   mat.translate(0, 0, 0);
		   }
	   }
	}
	bGL_PIXEL_UNPACK_BUFFER_BINDING = false;
}

void GLDrawClass::SaveToPixmap(void)
{
	if(save_pixmap_req) {
		if(!filename_screen_pixmap.isEmpty()) {
			QImage snapshot = this->grabFrameBuffer();
			snapshot.save(filename_screen_pixmap);
		}
		save_pixmap_req = false;
		filename_screen_pixmap.clear();
	}
}
