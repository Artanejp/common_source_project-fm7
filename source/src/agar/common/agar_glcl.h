/*
 * Header for CL with GL
 * (C) 2012 K.Ohta
 * Notes:
 *   Not CL model: VramDraw->[Virtual Vram]->AGEventDraw2->drawUpdateTexture->[GL Texture]->Drawing
 *       CL Model: AGEvenDraw2 -> GLCL_DrawEventSub -> [GL/CL Texture] ->Drawing
 * History:
 *   Nov 01,2012: Initial.
 */
#include <SDL/SDL.h>
#include <agar/core.h>

#ifdef _WINDOWS
#include <GL/gl.h>
#include <GL/glext.h>
#else
#include <GL/glx.h>
#include <GL/glxext.h>
#endif

#ifdef _USE_OPENCL
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include <CL/cl_gl_ext.h>

#if 1
#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#endif

extern "C" {
   #include "xm7_types.h"
   extern BYTE     ttl_palet[8];
   extern BYTE     apalet_b[4096];
   extern BYTE     apalet_r[4096];
   extern BYTE     apalet_g[4096];
   extern BYTE     multi_page;
};

extern GLuint uVramTextureID;

struct apalettetbl_t {
   Uint8 line_h;
   Uint8 line_l;
   Uint8 mpage;
   Uint8 r_4096[4096];
   Uint8 g_4096[4096];
   Uint8 b_4096[4096];
} __attribute__((packed));

struct dpalettetbl_t {
   Uint8 line_h;
   Uint8 line_l;
   Uint8 mpage;
   Uint8 tbl[8];
}__attribute__((packed));

struct palettebuf_t {
   Uint8 alines_h;
   Uint8 alines_l;
   Uint8 dlines_h;
   Uint8 dlines_l;
   struct apalettetbl_t atbls[200];
   struct dpalettetbl_t dtbls[400];
}__attribute__((packed));


class GLCLDraw {
 public:
   GLCLDraw();
   ~GLCLDraw();
   cl_int GetVram(int bmode);
   cl_int BuildFromSource(const char *p);
   cl_int SetupBuffer(GLuint *texid);
   cl_int SetupTable(void);
   cl_int InitContext(int platformnum, int processornum, int GLinterop);
   int GetPlatforms(void);
   int GetUsingDeviceNo(void);
   int GetDevices(void);
   void GetDeviceType(char *str, int maxlen, int num);
   void GetDeviceName(char *str, int maxlen, int num);
   Uint8 *MapTransferBuffer(int bmode);
   cl_int UnMapTransferBuffer(Uint8 *p);
   GLuint GetPbo(void);
   int GetGLEnabled(void);
   Uint32 *GetPixelBuffer(void);
   int ReleasePixelBuffer(Uint32 *p);
   Uint8 *GetBufPtr(Uint32 timeout);
   void ReleaseBufPtr(void);
   void AddPalette(int line, Uint8 mpage, BOOL analog);
   void ResetPalette(void);
   void CopyPalette(void);
   cl_context context = NULL;
   cl_command_queue command_queue = NULL;

   /* Program Object */
   const char *source = NULL;
   cl_program program = NULL;
   cl_int ret_num_devices;
   cl_int ret_num_platforms;
   cl_int platform_num = 0;
   cl_platform_id platform_id[8];
   cl_device_id device_id[8];

 private:
   CL_CALLBACK LogProgramExecute(cl_program program, void *userdata);
   CL_CALLBACK (*build_callback)(cl_program, void *);
   int w2 = 0;
   int h2 = 0;
   cl_event event_exec;
   cl_event event_uploadvram[4];
   cl_event event_copytotexture;
   cl_event event_release;
   cl_kernel kernels_array[16];
   cl_kernel *kernel_8colors = NULL;
   cl_kernel *kernel_4096colors = NULL;
   cl_kernel *kernel_256kcolors = NULL;
   cl_kernel *kernel_table = NULL; 
   cl_kernel *kernel_copyvram = NULL;
   cl_uint nkernels;

   int inbuf_bank = 0;
   int palette_bank = 0;
   int palette_bank_old = 0;
   cl_mem inbuf[2] = {NULL, NULL};
   cl_mem palette_buf[2] = {NULL, NULL};
   cl_mem outbuf = NULL;
   cl_mem internalpal = NULL;
   cl_mem table = NULL;
   cl_context_properties *properties = NULL;	
   GLuint pbo = 0;
   int lastline;
   int using_device = 0;
   int bCLEnableKhrGLShare = 0;
   Uint32 *pixelBuffer = NULL;
   Uint8 *TransferBuffer = NULL;
   struct palettebuf_t *palettebuf = NULL;
   int bModeOld = -1;
   cl_device_type device_type[8];
   cl_ulong local_memsize[8];
   AG_Mutex mutex_buffer;
   AG_Mutex mutex_palette;
};

enum {
  CLKERNEL_8 = 0,
  CLKERNEL_4096,
  CLKERNEL_256K,
  CLKERNEL_END
};

#endif /* _USE_OPENCL */
