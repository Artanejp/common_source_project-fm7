/*
 * Agar: OpenGLUtils
 * (C) 2011 K.Ohta <whatisthis.sowhat@gmail.com>
 */


#include "agar_glutil.h"
#ifdef _USE_OPENCL
#include "agar_glcl.h"
#endif
#include "agar_logger.h"

#ifdef USE_OPENMP
#include <omp.h>
#endif //_OPENMP

extern "C" {
    AG_GLView *GLDrawArea;
    BOOL bInitCL = FALSE;
    BOOL bCLEnabled = FALSE;
    BOOL bCLGLInterop = FALSE;
    int nCLGlobalWorkThreads = 10;
    BOOL bCLSparse = FALSE; // TRUE=Multi threaded CL,FALSE = Single Thread.
    int nCLPlatformNum;
    int nCLDeviceNum;
    BOOL bCLInteropGL;
    extern BOOL bUseOpenCL;
}

GLfloat GridVertexs200l[202 * 6];
GLfloat GridVertexs400l[402 * 6];

// Brights
extern float fBrightR;
extern float fBrightG;
extern float fBrightB;
extern const char *cl_render;
extern GLuint uVramTextureID;

#ifdef _USE_OPENCL
class GLCLDraw *cldraw = NULL;
#endif



GLuint CreateNullTexture(int w, int h)
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

GLuint CreateNullTextureCL(int w, int h)
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


void Flip_AG_GL(void)
{
	if(!InitVideo) return;
}

void DiscardTextures(int n, GLuint *id)
{
	if(GLDrawArea == NULL) return;
	if(agDriverOps == NULL) return;
	glDeleteTextures(n, id);

}

void DiscardTexture(GLuint tid)
{
	DiscardTextures(1, &tid);
}


void InitContextCL(void)
{
  if(GLDrawArea == NULL) return; // Context not created yet.
  if(bInitCL == TRUE) return; // CL already initialized.

#ifdef _USE_OPENCL
     bCLEnabled = FALSE;
     bCLGLInterop = FALSE;
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
		      bCLGLInterop = TRUE;
		      bCLEnabled = TRUE;
		    } else {
		      /*
		       *
		       */
		      bCLGLInterop = FALSE;
		      bCLEnabled = TRUE;
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
     bCLEnabled = FALSE;
     bCLGLInterop = FALSE;
#endif // _USE_OPENCL   
     bInitCL = TRUE;
}


static void InitGridVertexsSub(GLfloat *p, int h)
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


void InitGridVertexs(void)
{
   InitGridVertexsSub(GridVertexs200l, 200);
   InitGridVertexsSub(GridVertexs400l, 400);
}


void InitGL_AG2(int w, int h)
{
	Uint32 flags;
	int bpp = 32;
	int rgb_size[3];
	char *ext;

	if(InitVideo) return;
    InitVideo = TRUE;

    vram_pb = NULL;
    vram_pg = NULL;
    vram_pr = NULL;
#ifdef _USE_OPENCL
   cldraw = NULL;
#endif
	flags = SDL_OPENGL | SDL_RESIZABLE;
    switch (bpp) {
         case 8:
             rgb_size[0] = 3;
             rgb_size[1] = 3;
             rgb_size[2] = 2;
             break;
         case 15:
         case 16:
             rgb_size[0] = 5;
             rgb_size[1] = 5;
             rgb_size[2] = 5;
             break;
         default:
             rgb_size[0] = 8;
             rgb_size[1] = 8;
             rgb_size[2] = 8;
             break;
     }
    /*
     * GL 拡張の取得 20110907-
     */
	InitVramSemaphore();
	uVramTextureID = 0;
	uNullTextureID = 0;
	pVram2 = NULL;
#ifdef _USE_OPENCL
        bInitCL = FALSE;
        nCLGlobalWorkThreads = 10;
        bCLSparse = FALSE; // TRUE=Multi threaded CL,FALSE = Single Thread.
	nCLPlatformNum = 0;
	nCLDeviceNum = 0;
	bCLInteropGL = FALSE;
        //bCLDirectMapping = FALSE;
#endif
	InitVirtualVram();
        //if(AG_UsingSDL(NULL)) {
	   InitFBO(); // 拡張の有無を調べてからFBOを初期化する。
	   // FBOの有無を受けて、拡張の有無変数を変更する（念のために）
	   InitGLExtensionVars();
	   InitGridVertexs(); // Grid初期化
	//}
   
    fBrightR = 1.0; // 輝度の初期化
    fBrightG = 1.0;
    fBrightB = 1.0;

    return;
}


extern "C" {
// OpenGL状態変数
BOOL bGL_ARB_IMAGING; // イメージ操作可能か？
BOOL bGL_ARB_COPY_BUFFER;  // バッファ内コピー（高速化！）サポート
BOOL bGL_EXT_INDEX_TEXTURE; // パレットモードに係わる
BOOL bGL_EXT_COPY_TEXTURE; // テクスチャ間のコピー
BOOL bGL_SGI_COLOR_TABLE; // パレットモード(SGI拡張)
BOOL bGL_SGIS_PIXEL_TEXTURE; // テクスチャアップデート用
BOOL bGL_EXT_PACKED_PIXEL; // PackedPixelを使ってアップデートを高速化？
BOOL bGL_EXT_VERTEX_ARRAY; // 頂点を配列化して描画を高速化
BOOL bGL_EXT_PALETTED_TEXTURE; // パレットモード（更に別拡張)
BOOL bGL_PIXEL_UNPACK_BUFFER_BINDING; // ピクセルバッファがあるか？

   
// FBO API
PFNGLVERTEXPOINTEREXTPROC glVertexPointerEXT;
PFNGLDRAWARRAYSEXTPROC glDrawArraysEXT;
PFNGLTEXCOORDPOINTEREXTPROC glTexCoordPointerEXT;
//#ifndef _WINDOWS
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;
//#endif

BOOL QueryGLExtensions(const char *str)
{
    char *ext;
    char *p;
    int i;
    int j;
    int k;
    int l;
    int ll;
//#ifndef _WINDOWS

    if(str == NULL) return FALSE;
    ll = strlen(str);
    if(ll <= 0) return FALSE;

    ext =(char *)glGetString(GL_EXTENSIONS);
    if(ext == NULL) return FALSE;
    l = strlen(ext);
    if(l <= 0) return FALSE;
    p = ext;
    for(i = 0; i < l ; ){
        int j = strcspn(p, " ");
        if((ll == j) && (strncmp(str, p, j) == 0)) {
            return TRUE;
        }
        p += (j + 1);
        i += (j + 1);
    }
//#endif
    return FALSE;
}

void InitGLExtensionVars(void)
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
    bGL_PIXEL_UNPACK_BUFFER_BINDING = TRUE;
    bCLEnabled = FALSE;
    bCLGLInterop = FALSE;
}

   
#ifdef _WINDOWS
#include <windef.h>
extern PROC WINAPI wglGetProcAddress(LPCSTR lpszProc);
//#else 
//extern void *glXGetProcAddress(const GLubyte *);
#endif
   
void InitFBO(void)
{
//#ifndef _WINDOWS // glx is for X11.
// Use SDL for wrapper. 20130128
    if(AG_UsingSDL(NULL)) {
       glVertexPointerEXT = (PFNGLVERTEXPOINTEREXTPROC)SDL_GL_GetProcAddress("glVertexPointerEXT");
       if(glVertexPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glDrawArraysEXT = (PFNGLDRAWARRAYSEXTPROC)SDL_GL_GetProcAddress("glDrawArraysEXT");
       if(glDrawArraysEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glTexCoordPointerEXT = (PFNGLTEXCOORDPOINTEREXTPROC)SDL_GL_GetProcAddress("glTexCoordPointerEXT");
       if(glTexCoordPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glBindBuffer = (PFNGLBINDBUFFERPROC)SDL_GL_GetProcAddress("glBindBuffer");
       if(glBindBuffer == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glBufferData = (PFNGLBUFFERDATAPROC)SDL_GL_GetProcAddress("glBufferData");
       if(glBufferData == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glGenBuffers = (PFNGLGENBUFFERSPROC)SDL_GL_GetProcAddress("glGenBuffers");
       if(glGenBuffers == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)SDL_GL_GetProcAddress("glDeleteBuffers");
       if(glDeleteBuffers == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
    } else { // glx, wgl
#ifndef _WINDOWS
       glVertexPointerEXT = (PFNGLVERTEXPOINTEREXTPROC)glXGetProcAddress((const GLubyte *)"glVertexPointerEXT");
       if(glVertexPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glDrawArraysEXT = (PFNGLDRAWARRAYSEXTPROC)glXGetProcAddress((const GLubyte *)"glDrawArraysEXT");
       if(glDrawArraysEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glTexCoordPointerEXT = (PFNGLTEXCOORDPOINTEREXTPROC)glXGetProcAddress((const GLubyte *)"glTexCoordPointerEXT");
       if(glTexCoordPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glBindBuffer = (PFNGLBINDBUFFERPROC)glXGetProcAddress((const GLubyte *)"glBindBuffer");
       if(glBindBuffer == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glBufferData = (PFNGLBUFFERDATAPROC)glXGetProcAddress((const GLubyte *)"glBufferData");
       if(glBufferData == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glGenBuffers = (PFNGLGENBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glGenBuffers");
       if(glGenBuffers == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)glXGetProcAddress((const GLubyte *)"glDeleteBuffers");
       if(glDeleteBuffers == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
#else
       glVertexPointerEXT = (PFNGLVERTEXPOINTEREXTPROC)wglGetProcAddress("glVertexPointerEXT");
       if(glVertexPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glDrawArraysEXT = (PFNGLDRAWARRAYSEXTPROC)wglGetProcAddress("glDrawArraysEXT");
       if(glDrawArraysEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glTexCoordPointerEXT = (PFNGLTEXCOORDPOINTEREXTPROC)wglGetProcAddress("glTexCoordPointerEXT");
       if(glTexCoordPointerEXT == NULL) bGL_EXT_VERTEX_ARRAY = FALSE;
       glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
       if(glBindBuffer == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
       if(glBufferData == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
       if(glGenBuffers == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
       glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
       if(glDeleteBuffers == NULL) bGL_PIXEL_UNPACK_BUFFER_BINDING = FALSE;
#endif // _WINDOWS    
    }
   
}

}
