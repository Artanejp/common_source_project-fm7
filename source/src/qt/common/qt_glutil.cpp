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
#if 0
	if(bInitCL == true) return; // CL already initialized.
# ifdef _USE_OPENCL
     bCLEnabled = false;
     bCLGLInterop = false;
     if(bUseOpenCL && (cldraw == NULL) && 
	bGL_PIXEL_UNPACK_BUFFER_BINDING) {
	    cl_int r;
	    cldraw = new GLCLDraw;
	    if(cldraw != NULL) {
	      r = cldraw->InitContext(nCLPlatformNum, nCLDeviceNum, bCLInteropGL);
	       if(r == CL_SUCCESS){
		 r = cldraw->BuildFromSource(cl_render);
		 XM7_DebugLog(XM7_LOG_DEBUG, "CL: Build KERNEL: STS = %d", r);
	         if(r == CL_SUCCESS) {
		    r = cldraw->SetupBuffer(&uVramTextureID);
		    r |= cldraw->SetupTable();
		    if(r != CL_SUCCESS){
		       delete cldraw;
		       cldraw = NULL;
		    } else if(cldraw->GetGLEnabled() != 0) {
		      bCLGLInterop = true;
		      bCLEnabled = true;
		    } else {
		      /*
		       *
		       */
		      bCLGLInterop = false;
		      bCLEnabled = true;
		    }
		 } else {
		    delete cldraw;
		    cldraw = NULL;
		 }
	       } else {
		  delete cldraw;
		  cldraw = NULL;
	       }
	    }
     }
#else
     bCLEnabled = false;
     bCLGLInterop = false;
#endif // _USE_OPENCL   
#endif
   bInitCL = true;
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
#if 0
	if(str == NULL) return false;
	ll = strlen(str);
	if(ll <= 0) return false;
	
	ext = (char *)(extfunc->glGetString(GL_EXTENSIONS));
	if(ext == NULL) return false;
	l = strlen(ext);
	if(l <= 0) return false;
	p = ext;
	for(i = 0; i < l ; ){
		int j = strcspn(p, " ");
		if((ll == j) && (strncmp(str, p, j) == 0)) {
			return true;
		}
		p += (j + 1);
		i += (j + 1);
	}
#endif
	return false;
}

void GLDrawClass::InitGLExtensionVars(void)
{
#if 0
	bGL_ARB_IMAGING = QueryGLExtensions("GL_ARB_imaging");
	bGL_ARB_COPY_BUFFER = QueryGLExtensions("GL_ARB_copy_buffer");
	bGL_EXT_INDEX_TEXTURE = QueryGLExtensions("GL_EXT_index_texture");
	bGL_EXT_COPY_TEXTURE = QueryGLExtensions("GL_EXT_copy_texture");
	bGL_SGI_COLOR_TABLE = QueryGLExtensions("GL_SGI_color_table");
	bGL_SGIS_PIXEL_TEXTURE = QueryGLExtensions("GL_SGIS_pixel_texture");
	bGL_EXT_PACKED_PIXEL = QueryGLExtensions("GL_EXT_packed_pixel");
	bGL_EXT_PALETTED_TEXTURE = QueryGLExtensions("GL_EXT_paletted_texture");
	bGL_EXT_VERTEX_ARRAY = QueryGLExtensions("GL_EXT_vertex_array");
#endif	
	//    bGL_PIXEL_UNPACK_BUFFER_BINDING = QueryGLExtensions("GL_pixel_unpack_buffer_binding");
	bGL_PIXEL_UNPACK_BUFFER_BINDING = true;
	bCLEnabled = false;
	bCLGLInterop = false;
}

   
void GLDrawClass::InitFBO(void)
{
	bGL_EXT_VERTEX_ARRAY = false;
#if defined(_USE_GLAPI_QT5_4) || defined(_USE_GLAPI_QT5_1)
//# if defined(Q_OS_WIN32)  
	extfunc = new QOpenGLFunctions;
//# else
//	extfunc = new QOpenGLFunctions_3_0;
//# endif
#if 1
	main_shader = new QOpenGLShaderProgram(this);
	if(main_shader != NULL) {
		main_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/vertex_shader.glsl");
		main_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/fragment_shader.glsl");
		main_shader->link();
		//main_shader->bind();
	}
#endif	
# if 0	
	if(extfunc->initializeOpenGLFunctions()) bGL_EXT_VERTEX_ARRAY = true;
# else
	extfunc->initializeOpenGLFunctions();
# endif
#elif defined(_USE_GLAPI_QT4_8) || defined(_USE_GLAPI_QT5_0)
	extfunc = new QGLFunctions;
	//if(extfunc->initializeOpenGLFunctions()) bGL_EXT_VERTEX_ARRAY = true;
#endif   
#if 1
	if(extfunc) {
		vertex_grid_horizonal = new QOpenGLVertexArrayObject;
		if(vertex_grid_horizonal != NULL) {
			if(vertex_grid_horizonal->create()) {
				vertex_grid_horizonal->bind();
				extfunc->glBufferData(GL_ARRAY_BUFFER, SCREEN_HEIGHT * 6 * sizeof(GLfloat),
									  glHorizGrids, GL_STATIC_DRAW);
				vertex_grid_horizonal->release();
			}
		}
		
		vertex_grid_vertical = new QOpenGLVertexArrayObject;
		if(vertex_grid_vertical != NULL) {
			if(vertex_grid_vertical->create()) {
				vertex_grid_vertical->bind();
				extfunc->glBufferData(GL_ARRAY_BUFFER, SCREEN_WIDTH * 6 * sizeof(GLfloat),
									  glVertGrids, GL_STATIC_DRAW);
				vertex_grid_vertical->release();
			}
		}
# if defined(USE_BUTTON)
		{
			int i;
			GLfloat Vertexs[4][3];
			for(i = 0; i < MAX_BUTTONS; i++) {
				fButtonX[i] = -1.0 + (float)(buttons[i].x * 2) / (float)SCREEN_WIDTH;
				fButtonY[i] = 1.0 - (float)(buttons[i].y * 2) / (float)SCREEN_HEIGHT;
				fButtonWidth[i] = (float)(buttons[i].width * 2) / (float)SCREEN_WIDTH;
				fButtonHeight[i] = (float)(buttons[i].height * 2) / (float)SCREEN_HEIGHT;
				Vertexs[0][2] = Vertexs[1][2] = Vertexs[2][2] = Vertexs[3][2] = 0.2f; // BG
				Vertexs[0][0] = Vertexs[3][0] = fButtonX[i]; // Xbegin
				Vertexs[0][1] = Vertexs[1][1] = fButtonY[i] ;  // Yend
				Vertexs[2][0] = Vertexs[1][0] = fButtonX[i] + fButtonWidth[i]; // Xend
				Vertexs[2][1] = Vertexs[3][1] = fButtonY[i] - fButtonHeight[i]; // Ybegin
				vertex_button[i] = new QOpenGLVertexArrayObject;
				if(vertex_button[i] != NULL) {
					if(vertex_button[i]->create()) {
						vertex_button[i]->bind();
						extfunc->glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat),
											  Vertexs, GL_STATIC_DRAW);
						vertex_button[i]->release();
					}
				}
			}
		}
#endif
#if defined(USE_BITMAP)
		vertex_bitmap = new QOpenGLVertexArrayObject;
		if(vertex_bitmap != NULL) {
			if(vertex_bitmap->create()) {
				vertex_bitmap->bind();
				extfunc->glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat),
									  BitmapVertexs, GL_STATIC_DRAW);
				vertex_bitmap->release();
			}
		}
#endif
		buffer_screen_vertex = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		buffer_screen_texture = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
		vertex_screen = new QOpenGLVertexArrayObject;
		if(vertex_screen != NULL) {
			if(vertex_screen->create()) {
				QVector<QVector3D> v;
				QVector<QVector2D> t;
				QVector<QVector4D> c;

				vertexFormat[0].x = -0.5f;
				vertexFormat[0].y = -0.5f;
				vertexFormat[0].z = 0.0f;
				vertexFormat[0].s = 0.0f;
				vertexFormat[0].t = 0.0f;
			   
				vertexFormat[1].x = +0.5f;
				vertexFormat[1].y = -0.5f;
				vertexFormat[1].z = 0.0f;
				vertexFormat[1].s = 1.0f;
				vertexFormat[1].t = 0.0f;
			   
				vertexFormat[2].x = +0.5f;
				vertexFormat[2].y = +0.5f;
				vertexFormat[2].z = 0.0f;
				vertexFormat[2].s = 1.0f;
				vertexFormat[2].t = 1.0f;
			   
				vertexFormat[3].x = -0.5f;
				vertexFormat[3].y = +0.5f;
				vertexFormat[3].z = 0.0f;
				vertexFormat[3].s = 0.0f;
				vertexFormat[3].t = 1.0f;
			   
				v << QVector3D(-0.5f, -0.5f, -0.0f);
				v << QVector3D(0.5f, -0.5f, -0.0f);
				v << QVector3D(0.5f, 0.5f, -0.0f);
				v << QVector3D(-0.5f, 0.5f, -0.0f);
				
				t << QVector2D(0.0f, 0.0f);
				t << QVector2D(1.0f, 0.0f);
				t << QVector2D(1.0f, 1.0f);
				t << QVector2D(1.0f, 0.0f);
				
				buffer_screen_vertex->create();
				buffer_screen_vertex->setUsagePattern(QOpenGLBuffer::DynamicDraw);
				int vertex_loc = main_shader->attributeLocation("vertex");
				int texcoord_loc = main_shader->attributeLocation("texcoord");

				vertex_screen->bind();
				buffer_screen_vertex->bind();
				buffer_screen_vertex->allocate(sizeof(vertexFormat));
				
				buffer_screen_vertex->write(0, vertexFormat, sizeof(vertexFormat));
				main_shader->setAttributeBuffer(vertex_loc, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
				main_shader->setAttributeBuffer(texcoord_loc, GL_FLOAT, 3, 2, 5 * sizeof(GLfloat));
				buffer_screen_vertex->release();
				vertex_screen->release();
				main_shader->enableAttributeArray(vertex_loc);
				main_shader->enableAttributeArray(texcoord_loc);
				
				
				main_shader->setUniformValue("a_texture", 0);
				
				extfunc->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0); 
				QMatrix4x4 mat;
				mat.ortho(-1.0, 1.0, -1.0, +1.0, -1.0, 1.0);
				mat.translate(0, 0, 0);
			}
		}
	}
#endif // if 1
	bGL_PIXEL_UNPACK_BUFFER_BINDING = false;
}
