#ifndef AGAR_GLUTIL_H_INCLUDED
#define AGAR_GLUTIL_H_INCLUDED


#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>
#include <agar/gui/glview.h>

#include <SDL/SDL.h>
#ifdef _WINDOWS
#include <GL/gl.h>
#include <GL/glext.h>
#include <SDL/SDL_opengl.h>
#else
#include <GL/glx.h>
#include <GL/glxext.h>
#include <SDL/SDL_opengl.h>
#endif

#include "api_draw.h"
#include "agar_xm7.h"
#include "agar_vramutil.h"
#include "agar_draw.h"
#include "agar_gldraw.h"

#ifdef __cplusplus
extern "C" {
#endif
extern  AG_GLView *GLDrawArea;
extern BOOL bGL_ARB_IMAGING; // イメージ操作可能か？
extern BOOL bGL_ARB_COPY_BUFFER;  // バッファ内コピー（高速化！）サポート
extern BOOL bGL_EXT_INDEX_TEXTURE; // パレットモードに係わる
extern BOOL bGL_EXT_COPY_TEXTURE; // テクスチャ間のコピー
extern BOOL bGL_SGI_COLOR_TABLE; // パレットモード(SGI拡張)
extern BOOL bGL_SGIS_PIXEL_TEXTURE; // テクスチャアップデート用
extern BOOL bGL_EXT_PACKED_PIXEL; // PackedPixelを使ってアップデートを高速化？
extern BOOL bGL_EXT_VERTEX_ARRAY; // 頂点を配列化して描画を高速化
extern BOOL bGL_EXT_PALETTED_TEXTURE; // パレットモード（更に別拡張)
extern BOOL bGL_PIXEL_UNPACK_BUFFER_BINDING; // Pixel buffer
extern BOOL bCLEnabled;
extern BOOL bCLGLInterop;

// FBO API
extern PFNGLVERTEXPOINTEREXTPROC glVertexPointerEXT;
extern PFNGLDRAWARRAYSEXTPROC glDrawArraysEXT;
extern PFNGLTEXCOORDPOINTEREXTPROC glTexCoordPointerEXT;
#ifndef _WINDOWS
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
#endif
   
extern void InitFBO(void);
   
extern void InitGLExtensionVars(void);
extern BOOL QueryGLExtensions(const char *str);

#ifdef _USE_OPENCL
extern int nCLGlobalWorkThreads;
extern BOOL bCLSparse; // TRUE=Multi threaded CL,FALSE = Single Thread.
extern BOOL bInitCL;
   //extern BOOL bCLDirectMapping;
#endif // _USE_OPENCL
#ifdef __cplusplus
}
#endif
#ifdef __cplusplus
extern GLuint CreateNullTexture(int w, int h);
extern GLuint CreateNullTextureCL(int w, int h);

extern void Flip_AG_GL(void);
extern void DiscardTextures(int n, GLuint *id);
extern void DiscardTexture(GLuint id);
#endif

#endif // AGAR_GLUTIL_H_INCLUDED
