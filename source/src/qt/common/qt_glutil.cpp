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
void GLDrawClass::update_screen()
{
	//if(tick < (1000 / 75)) tick = 1000 / 75;
	if(p_emu == NULL) return;
	imgptr = p_emu->getPseudoVramClass();
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
	return false;
}

void GLDrawClass::InitGLExtensionVars(void)
{
	bGL_ARB_IMAGING = QueryGLExtensions("GL_ARB_imaging");
	bGL_ARB_COPY_BUFFER = QueryGLExtensions("GL_ARB_copy_buffer");
	bGL_EXT_INDEX_TEXTURE = QueryGLExtensions("GL_EXT_index_texture");
	bGL_EXT_COPY_TEXTURE = QueryGLExtensions("GL_EXT_copy_texture");
	bGL_SGI_COLOR_TABLE = QueryGLExtensions("GL_SGI_color_table");
	bGL_SGIS_PIXEL_TEXTURE = QueryGLExtensions("GL_SGIS_pixel_texture");
	bGL_EXT_PACKED_PIXEL = QueryGLExtensions("GL_EXT_packed_pixel");
	bGL_EXT_PALETTED_TEXTURE = QueryGLExtensions("GL_EXT_paletted_texture");
	bGL_EXT_VERTEX_ARRAY = QueryGLExtensions("GL_EXT_vertex_array");
	//    bGL_PIXEL_UNPACK_BUFFER_BINDING = QueryGLExtensions("GL_pixel_unpack_buffer_binding");
	bGL_PIXEL_UNPACK_BUFFER_BINDING = true;
	bCLEnabled = false;
	bCLGLInterop = false;
}

   
void GLDrawClass::InitFBO(void)
{
	bGL_EXT_VERTEX_ARRAY = false;
#if defined(_USE_GLAPI_QT5_4) || defined(_USE_GLAPI_QT5_1)
	extfunc = new QOpenGLFunctions_2_0;
	if(extfunc->initializeOpenGLFunctions()) bGL_EXT_VERTEX_ARRAY = true;
#elif defined(_USE_GLAPI_QT4_8) || defined(_USE_GLAPI_QT5_0)
	extfunc = new QGLFunctions;
	//if(extfunc->initializeOpenGLFunctions()) bGL_EXT_VERTEX_ARRAY = true;
#endif   
	bGL_PIXEL_UNPACK_BUFFER_BINDING = false;
}
