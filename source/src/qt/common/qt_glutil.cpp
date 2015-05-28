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

//extern const char *cl_render;

#ifdef _USE_OPENCL
//class GLCLDraw *cldraw = NULL;
#endif
void GLDrawClass::update_screen(int tick)
{
	//if(tick < (1000 / 75)) tick = 1000 / 75;
	updateGL();
	printf("UpdateGL(), %d\n", SDL_GetTicks());
	//timer->start(tick);
	//emit update_screenChanged(tick);
}


GLuint GLDrawClass::CreateNullTexture(int w, int h)
{
	GLuint ttid;
	Uint32 *p;

	p =(Uint32 *)malloc((w + 2)*  (h  + 2) * sizeof(Uint32));
	if(p == NULL) return 0;

	//    memset(p, 0x00, (w + 2) * (h + 2) * sizeof(Uint32));
	memset(p, 0x00, (w + 2) * (h + 2) * sizeof(Uint32));
	glGenTextures(1, &ttid);
	glBindTexture(GL_TEXTURE_2D, ttid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1); // Limit mipmap level , reduce resources.
	glTexImage2D(GL_TEXTURE_2D,
		     0,
		     GL_RGBA,
		     w, h + 2,
		     0,
		     GL_RGBA,
		     GL_UNSIGNED_BYTE,
		     p);
	free(p);
	return ttid;
}

GLuint GLDrawClass::CreateNullTextureCL(int w, int h)
{
	GLuint ttid;
	Uint32 *p;

	p =(Uint32 *)malloc((w + 2)*  (h  + 2) * sizeof(Uint32));
	if(p == NULL) return 0;
	memset(p, 0x00, (w + 2) * (h + 2) * sizeof(Uint32));
	glGenTextures(1, &ttid);
	glBindTexture(GL_TEXTURE_2D, ttid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1); // Limit mipmap level , reduce resources.
	glTexImage2D(GL_TEXTURE_2D,
		     0,
		     GL_RGBA8UI,
		     w, h + 2,
		     0,
		     GL_RGBA_INTEGER,
		     GL_UNSIGNED_BYTE,
		     p);
	glBindTexture(GL_TEXTURE_2D, 0);
	free(p);
	return ttid;
}

void GLDrawClass::DiscardTextures(int n, GLuint *id)
{
	glDeleteTextures(n, id);
}

void GLDrawClass::DiscardTexture(GLuint tid)
{
	DiscardTextures(1, &tid);
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


void GLDrawClass::InitGridVertexsSub(GLfloat *p, int h)
{
	int y;
	int yp;
	float yf;
	yp = 0;
	for(y = 0; y < (h + 1); y++) {
		yf = -1.0f + (float) (y + 1) * 2.0f / (float)h;
		p[yp + 0] = -1.0f;
		p[yp + 1] = yf;
		p[yp + 2] = 0.96f;
		p[yp + 3] = -1.0f;
		p[yp + 4] = yf;
		p[yp + 5] = 0.96f;
		yp += 6;
	}
	return;
}


void GLDrawClass::InitGridVertexs(void)
{
	InitGridVertexsSub(GridVertexs200l, 200);
	InitGridVertexsSub(GridVertexs400l, 400);
}

void GLDrawClass::initializeGL(void)
{
#ifdef _USE_OPENCL
	cldraw = NULL;
#endif
	/*
	 * GL 拡張の取得 20110907-
	 */
	//InitVramSemaphore();
	//	InitVirtualVram();
        //if(AG_UsingSDL(NULL)) {
	InitFBO(); // 拡張の有無を調べてからFBOを初期化する。
	// FBOの有無を受けて、拡張の有無変数を変更する（念のために）
	InitGLExtensionVars();
	InitGridVertexs(); // Grid初期化
	// Init view
	glClearColor(0.0, 0.0, 0.0, 1.0);
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
	
	ext =(char *)glGetString(GL_EXTENSIONS);
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

   
#ifdef _WINDOWS
#include <windef.h>
extern PROC WINAPI wglGetProcAddress(LPCSTR lpszProc);
//#else 
//extern void *glXGetProcAddress(const GLubyte *);
#endif
   
void GLDrawClass::InitFBO(void)
{
	//#ifndef _WINDOWS // glx is for X11.
	// Use SDL for wrapper. 20130128
	//glVertexPointerEXT = (PFNGLVERTEXPOINTEREXTPROC)SDL_GL_GetProcAddress("glVertexPointerEXT");
	bGL_EXT_VERTEX_ARRAY = false;
	//glDrawArraysEXT = (PFNGLDRAWARRAYSEXTPROC)SDL_GL_GetProcAddress("glDrawArraysEXT");
	bGL_EXT_VERTEX_ARRAY = false;
	//glTexCoordPointerEXT = (PFNGLTEXCOORDPOINTEREXTPROC)SDL_GL_GetProcAddress("glTexCoordPointerEXT");
	bGL_EXT_VERTEX_ARRAY = false;
	//glBindBuffer = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
	bGL_PIXEL_UNPACK_BUFFER_BINDING = false;
	//glBufferData = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
	bGL_PIXEL_UNPACK_BUFFER_BINDING = false;
	//glGenBuffers = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
	bGL_PIXEL_UNPACK_BUFFER_BINDING = false;
	//glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
	bGL_PIXEL_UNPACK_BUFFER_BINDING = false;
}
