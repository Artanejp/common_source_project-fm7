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

//extern const char *cl_render;

#ifdef _USE_OPENCL
//class GLCLDraw *cldraw = NULL;
#endif
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
	glHorizGrids = (GLfloat *)malloc(sizeof(float) * (SCREEN_HEIGHT + 2) * 6);
	if(glHorizGrids != NULL) {
		yf = -1.0f;
		delta = 2.0f / (float)SCREEN_HEIGHT;
		yf = yf - delta * 0.75f;
		for(i = 0; i < (SCREEN_HEIGHT + 1) ; i++) {
			glHorizGrids[i * 6]     = -1.0f; // XBegin
			glHorizGrids[i * 6 + 3] = +1.0f; // XEnd
			glHorizGrids[i * 6 + 1] = yf; // YBegin
			glHorizGrids[i * 6 + 4] = yf; // YEnd
			glHorizGrids[i * 6 + 2] = 0.1f; // ZBegin
			glHorizGrids[i * 6 + 5] = 0.1f; // ZEnd
			yf = yf + delta;
		}
	}
	glVertGrids  = (GLfloat *)malloc(sizeof(float) * (SCREEN_WIDTH + 2) * 6);
	if(glVertGrids != NULL) {
		xf = -1.0f; 
		delta = 2.0f / (float)SCREEN_WIDTH;
		xf = xf - delta * 0.75f;
		for(i = 0; i < (SCREEN_WIDTH + 1) ; i++) {
			glVertGrids[i * 6]     = xf; // XBegin
			glVertGrids[i * 6 + 3] = xf; // XEnd
			glVertGrids[i * 6 + 1] = -1.0f; // YBegin
			glVertGrids[i * 6 + 4] =  1.0f; // YEnd
			glVertGrids[i * 6 + 2] = 0.1f; // ZBegin
			glVertGrids[i * 6 + 5] = 0.1f; // ZEnd
			xf = xf + delta;
		}
	}
	
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

   
void GLDrawClass::InitFBO(void)
{
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
		main_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragment_shader.glsl");
		main_shader->link();
	}
	grids_shader = new QOpenGLShaderProgram(this);
	if(grids_shader != NULL) {
		grids_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grids_vertex_shader.glsl");
		grids_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grids_fragment_shader.glsl");
		grids_shader->link();
	}

# if defined(USE_BITMAP)
   	bitmap_shader = new QOpenGLShaderProgram(this);
	if(bitmap_shader != NULL) {
		bitmap_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
		bitmap_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragment_shader.glsl");
		bitmap_shader->link();
	}
# endif
# if defined(USE_BUTTON)
   	button_shader = new QOpenGLShaderProgram(this);
	if(button_shader != NULL) {
		button_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
		button_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragment_shader.glsl");
		button_shader->link();
	}
# endif
	if(extfunc) {
		buffer_grid_horizonal = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		vertex_grid_horizonal = new QOpenGLVertexArrayObject;
		QVector3D hg;
		if(vertex_grid_horizonal != NULL) {
			if(vertex_grid_horizonal->create()) {
				vertex_grid_horizonal->bind();
				buffer_grid_horizonal->allocate(SCREEN_HEIGHT * 6 * sizeof(GLfloat));
				vertex_grid_horizonal->release();
			}
		}
		
		buffer_grid_vertical = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		vertex_grid_vertical = new QOpenGLVertexArrayObject;
		if(vertex_grid_vertical != NULL) {
			if(vertex_grid_vertical->create()) {
				vertex_grid_vertical->bind();
				buffer_grid_horizonal->allocate(SCREEN_WIDTH * 6 * sizeof(GLfloat));
				vertex_grid_vertical->release();
			}
		}
# if defined(USE_BUTTON)
		{
			int i;
			GLfloat Vertexs[4][3];
			for(i = 0; i < MAX_BUTTONS; i++) {
				buffer_button_vertex[i] = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
				fButtonX[i] = -1.0 + (float)(buttons[i].x * 2) / (float)SCREEN_WIDTH;
				fButtonY[i] = 1.0 - (float)(buttons[i].y * 2) / (float)SCREEN_HEIGHT;
				fButtonWidth[i] = (float)(buttons[i].width * 2) / (float)SCREEN_WIDTH;
				fButtonHeight[i] = (float)(buttons[i].height * 2) / (float)SCREEN_HEIGHT;
			   
				vertex_button[i] = new QOpenGLVertexArrayObject;
				if(vertex_button[i] != NULL) {
					if(vertex_button[i]->create()) {
					VertexTexCoord_t vt;
					   vt[0].x =  fButtonX[i];
					   vt[0].y =  fButtonY[i];
					   vt[0].z =  -0.2f;
					   vt[0].s = 0.0f;
					   vt[0].t = 1.0f;
					   
					   vt[1].x =  fButtonX[i] + fButtonWidth[i];
					   vt[1].y =  fButtonY[i];
					   vt[1].z =  -0.2f;
					   vt[1].s = 1.0f;
					   vt[1].t = 1.0f;
					   
					   vt[2].x =  fButtonX[i] + fButtonWidth[i];
					   vt[2].y =  fButtonY[i] - fButtonHeight[i];
					   vt[2].z =  -0.2f;
					   vt[2].s = 1.0f;
					   vt[2].t = 0.0f;
					   
					   vt[3].x =  fButtonX[i];
					   vt[3].y =  fButtonY[i] - fButtonHeight[i];
					   vt[3].z =  -0.2f;
					   vt[3].s = 0.0f;
					   vt[3].t = 0.0f;
		   
					   buffer_button_vertex->write(0, vt, 4 * sizeof(VertexTexCoord_t));
					   buffer_button_vertex[i]->create();
					   buffer_button_vertex[i]->setUsagePattern(QOpenGLBuffer::DynamicDraw);
					   int vertex_loc = main_shader->attributeLocation("vertex");
					   int texcoord_loc = main_shader->attributeLocation("texcoord");
					   
					   vertex_button[i]->bind();
					   buffer_button_vertex[i]->bind();
					   buffer_button_vertex[i]->allocate(sizeof(vt));
				
					   buffer_button_vertex[i]->write(0, vertexFormat, sizeof(vertexFormat));
					   button_shader->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
					   button_shader->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));
					   buffer_button_vertex[i]->release();
					   vertex_button[i]->release();
					   button_shader->setUniformValue("a_texture", 0);
			
					   extfunc->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0); 
					   extfunc->glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 
									  (char *)NULL + 3 * sizeof(GLfloat)); 
					   button_shader->enableAttributeArray(vertex_loc);
					   button_shader->enableAttributeArray(texcoord_loc);
					   vertex_button[i]->release();
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
			   vertexBitmap[0].x = -1.0f;
			   vertexBitmap[0].y = -1.0f;
			   vertexBitmap[0].z = -0.9f;
			   vertexBitmap[0].s = 0.0f;
			   vertexBitmap[0].t = 1.0f;
			   
			   vertexBitmap[1].x = +1.0f;
			   vertexBitmap[1].y = -1.0f;
			   vertexBitmap[1].z = -0.9f;
			   vertexBitmap[1].s = 1.0f;
			   vertexBitmap[1].t = 1.0f;
			   
			   vertexBitmap[2].x = +1.0f;
			   vertexBitmap[2].y = +1.0f;
			   vertexBitmap[2].z = -0.9f;
			   vertexBitmap[2].s = 1.0f;
			   vertexBitmap[2].t = 0.0f;
			   
			   vertexBitmap[3].x = -1.0f;
			   vertexBitmap[3].y = +1.0f;
			   vertexBitmap[3].z = -0.9f;
			   vertexBitmap[3].s = 0.0f;
			   vertexBitmap[3].t = 0.0f;
			   
			   buffer_bitmap_vertex->create();
			   buffer_bitmap_vertex->setUsagePattern(QOpenGLBuffer::DynamicDraw);
			   int vertex_loc = main_shader->attributeLocation("vertex");
			   int texcoord_loc = main_shader->attributeLocation("texcoord");
			   
			   vertex_bitmap->bind();
			   buffer_bitmap_vertex->bind();
			   buffer_bitmap_vertex->allocate(sizeof(vertexBitmap));
			   
			   buffer_bitmap_vertex->write(0, vertexFormat, sizeof(vertexFormat));
			   bitmap_shader->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
			   bitmap_shader->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));
			   buffer_bitmap_vertex->release();
			   vertex_bitmap->release();
			   bitmap_shader->setUniformValue("a_texture", 0);
			   
			   extfunc->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0); 
			   extfunc->glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 
											  (char *)NULL + 3 * sizeof(GLfloat)); 
			   bitmap_shader->enableAttributeArray(vertex_loc);
			   bitmap_shader->enableAttributeArray(texcoord_loc);
			   vertex_bitmap->release();
		   }
		}
#endif
	   buffer_screen_vertex = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
	   vertex_screen = new QOpenGLVertexArrayObject;
	   if(vertex_screen != NULL) {
		   if(vertex_screen->create()) {
			   vertexFormat[0].x = -0.5f;
			   vertexFormat[0].y = -0.5f;
			   vertexFormat[0].z = 0.0f;
			   vertexFormat[0].s = 0.0f;
			   vertexFormat[0].t = 1.0f;
			   
			   vertexFormat[1].x = +0.5f;
			   vertexFormat[1].y = -0.5f;
			   vertexFormat[1].z = 0.0f;
			   vertexFormat[1].s = 1.0f;
			   vertexFormat[1].t = 1.0f;
			   
			   vertexFormat[2].x = +0.5f;
			   vertexFormat[2].y = +0.5f;
			   vertexFormat[2].z = 0.0f;
			   vertexFormat[2].s = 1.0f;
			   vertexFormat[2].t = 0.0f;
			   
			   vertexFormat[3].x = -0.5f;
			   vertexFormat[3].y = +0.5f;
			   vertexFormat[3].z = 0.0f;
			   vertexFormat[3].s = 0.0f;
			   vertexFormat[3].t = 0.0f;
			   
			   
			   buffer_screen_vertex->create();
			   buffer_screen_vertex->setUsagePattern(QOpenGLBuffer::DynamicDraw);
			   int vertex_loc = main_shader->attributeLocation("vertex");
			   int texcoord_loc = main_shader->attributeLocation("texcoord");
			   
			   vertex_screen->bind();
			   buffer_screen_vertex->bind();
			   buffer_screen_vertex->allocate(sizeof(vertexFormat));
			   
			   buffer_screen_vertex->write(0, vertexFormat, sizeof(vertexFormat));
			   main_shader->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
			   main_shader->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));
			   buffer_screen_vertex->release();
			   vertex_screen->release();
			   main_shader->setUniformValue("a_texture", 0);
			   
			   extfunc->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0); 
			   extfunc->glVertexAttribPointer(texcoord_loc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 
							       (char *)NULL + 3 * sizeof(GLfloat)); 
			   main_shader->enableAttributeArray(vertex_loc);
			   main_shader->enableAttributeArray(texcoord_loc);
			   QMatrix4x4 mat;
			   mat.ortho(-1.0, 1.0, -1.0, +1.0, -1.0, 1.0);
			   mat.translate(0, 0, 0);
		   }
	   }
	}
	bGL_PIXEL_UNPACK_BUFFER_BINDING = false;
}
