class GLDrawClass: public QGLWidget 
{
   Q_OBJECT
 protected:
     GLuint uVramTextureID;
     GLuint uNullTextureID;
     float GridVertexs200l[202 * 6];
     float GridVertexs400l[402 * 6];
     float fBrightR;
     float fBrightG;
     float fBrightB;
     // Will move to OpenCL
     bool bInitCL;
     bool bCLEnabled;
     bool bCLGLInterop;
     int nCLGlobalWorkThreads;
     bool bCLSparse; // TRUE=Multi threaded CL,FALSE = Single Thread.
     int nCLPlatformNum;
     int nCLDeviceNum;
     bool bCLInteropGL;
     //
     bool InitVideo;
     void drawGrids(void *pg,int w, int h);
     void GLDrawClass::InitGL(void);

#ifdef _USE_OPENCL
//     extern class GLCLDraw *cldraw;
#endif
     bool QueryGLExtensions(const char *str)
     void InitGLExtensionVars(void);
     void InitGridVertexsSub(GLfloat *p, int h);
     void InitContextCL(void);

 public:
     bool crt_flag;
     GLuint CreateNullTexture(int w, int h);
     GLuint CreateNullTextureCL(int w, int h);
     GLDrawClass( QWidget *parent = 0 , const char *name = 0 ) {
        InitGL();
        InitGLExtensionVars();
#ifdef _USE_OPENCL
	// Init CL
#endif
	
     }
   
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

     PFNGLBINDBUFFERPROC glBindBuffer;
     PFNGLBUFFERDATAPROC glBufferData;
     PFNGLGENBUFFERSPROC glGenBuffers;
     PFNGLDELETEBUFFERSPROC glDeleteBuffers;
     SetBrightRGB(float r, float g, float b);
     void drawUpdateTexture(Uint32 *p, int w, int h, BOOL crtflag);
     void DrawHandler(void);
     void InitFBO(void);
     void InitGridVertexs(void);
     void DiscardTextures(int n, GLuint *id);
     void DiscardTexture(GLuint tid);
}
