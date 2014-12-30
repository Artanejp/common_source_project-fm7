/*
 * Renderer using OPENCL/GL
 * (C) 2012 K.Ohta <whatisthis.sowhat@gmail.com>
 * History:
 * Nov 01,2012: Initial
 */


#include <agar/core.h>
#include <agar/core/types.h>
#include <agar/gui.h>


#include "api_draw.h"
#include "api_kbd.h"

#include "agar_xm7.h"
#include "agar_draw.h"
#include "agar_gldraw.h"
#include "agar_glutil.h"
#include "agar_logger.h"
#include "xm7.h"
#include "display.h"
#include "subctrl.h"
#include "device.h"
#include "multipag.h"
#include "ttlpalet.h"
#include "apalet.h"

#include "agar_glcl.h"

#define LOGSIZE 1024*1024

extern "C"{
extern Uint8 *vram_pb;
extern Uint8 *vram_pr;
extern Uint8 *vram_pg;
}

extern PFNGLBINDBUFFERPROC glBindBuffer;
extern float fBrightR;
extern float fBrightG;
extern float fBrightB;



GLCLDraw::GLCLDraw()
{
   int i;
   pixelBuffer = NULL;
   AG_MutexInit(&mutex_buffer);
   AG_MutexInit(&mutex_palette);
   TransferBuffer = NULL;
   nkernels = 0;
   using_device = 0;
   for(i = 0; i < 8; i++) device_type[i] = 0;
   for(i = 0; i < 8; i++) local_memsize[i] = 0;
}

GLCLDraw::~GLCLDraw()
{
   cl_int ret;
   int i;
   if(nkernels > 0) {
     for(i = 0; i < nkernels; i++) if(kernels_array[i] != NULL) ret = clReleaseKernel(kernels_array[i]);
   }
   
   if(program != NULL) ret |= clReleaseProgram(program);
   if(command_queue != NULL) ret |= clReleaseCommandQueue(command_queue);
   if(context != NULL) ret |= clReleaseContext(context);
   if(properties != NULL) free(properties);
   for(i = 0; i < 2; i++) {
      if(inbuf[i] != NULL) ret |= clReleaseMemObject(inbuf[i]);
      if(palette_buf[i] != NULL) ret |= clReleaseMemObject(palette_buf[i]);
   }
   if(outbuf != NULL) ret |= clReleaseMemObject(outbuf);
   if(table != NULL) ret |= clReleaseMemObject(table);
   if(pixelBuffer != NULL) free(pixelBuffer);
   AG_MutexDestroy(&mutex_buffer);
   AG_MutexDestroy(&mutex_palette);
}

static void cl_notify_log(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
   Uint8 dump[128];
   char dumpStr[1024];
   int i;
   
   dumpStr[0] = '\0';
   XM7_DebugLog(XM7_LOG_WARN, "CL Notify: %s", errinfo);
}

int GLCLDraw::GetGLEnabled(void)
{
  if(bCLEnableKhrGLShare != FALSE) return -1;
  return 0;
}

Uint32 *GLCLDraw::GetPixelBuffer(void)
{
    Uint32 *p;
    int ret = 0;
    p = (Uint32 *) clEnqueueMapBuffer(command_queue, outbuf, CL_TRUE, CL_MAP_READ,
				       0, (size_t)(640 * 400 * sizeof(Uint32)),
				       1, &event_exec, &event_release, &ret);
    if(ret < 0) return NULL;
    clFlush(command_queue);
    return p;
}

int GLCLDraw::ReleasePixelBuffer(Uint32 *p)
{
#if 0
   return 0;
#else
  int ret;
  if(p == NULL) return 0;
//  clFlush(command_queue);
  ret |= clEnqueueUnmapMemObject(command_queue, outbuf,
				 p, 1, &event_release, NULL);
  clFinish(command_queue);
  return ret;
#endif
}

int GLCLDraw::GetUsingDeviceNo(void)
{
  return using_device;
}

int GLCLDraw::GetDevices(void)
{
  return ret_num_devices;
}

int GLCLDraw::GetPlatforms(void)
{
  return ret_num_platforms;
}

void GLCLDraw::GetDeviceType(char *str, int maxlen, int num)
{
  if((str == NULL) || (maxlen < 1)) return;
  str[0] = '\0';
  if((num < 0) || (num >= 8) || (num >= ret_num_devices)) return;

  switch(device_type[num]) {
  case CL_DEVICE_TYPE_CPU:
       strncpy(str, "CPU", maxlen - 1);
       break;
  case CL_DEVICE_TYPE_GPU:
       strncpy(str, "GPU", maxlen - 1);
       break;
  case CL_DEVICE_TYPE_ACCELERATOR:
       strncpy(str, "ACCELERATOR", maxlen - 1);
       break;
  case CL_DEVICE_TYPE_DEFAULT:
       strncpy(str, "DEFAULT", maxlen - 1);
       break;
  default:
       strncpy(str, "Unknown", maxlen - 1);
       break;
     }
}

void GLCLDraw::GetDeviceName(char *str, int maxlen, int num)
{
  size_t llen;

  if((str == NULL) || (maxlen < 1)) return;
  str[0] = '\0';
  if((num < 0) || (num > 8) || (num >= ret_num_devices)) return;
  clGetDeviceInfo(device_id[num], CL_DEVICE_NAME,
		     maxlen - 1, str, &llen);
  str[llen - 1] = '\0';
}

cl_int GLCLDraw::InitContext(int platformnum, int processornum, int GLinterop)
{
   cl_int ret;
   size_t len;
   char extension_data[1024];
   size_t llen;
   size_t extension_len;
   int i;
   
   properties = malloc(16 * sizeof(intptr_t));
   ret = clGetPlatformIDs(8, platform_id, &ret_num_platforms);
   if(ret != CL_SUCCESS) return ret;

   if(ret_num_platforms <= 0) return CL_INVALID_PLATFORM;

   platform_num = platformnum;
   if(platform_num >= ret_num_platforms) platform_num = ret_num_platforms - 1;
   if(platform_num <= 0) platform_num = 0;
   ret = clGetDeviceIDs(platform_id[platform_num], CL_DEVICE_TYPE_ALL, 8, device_id,
                            &ret_num_devices);
   if(ret != CL_SUCCESS) return ret;
   if(ret_num_devices <= 0) {
     XM7_DebugLog(XM7_LOG_DEBUG, "CL : Has no useful device(s).");
     return ret;
   }
   if(ret_num_devices > 8) ret_num_devices = 8;
   if(ret_num_devices <= 0) return CL_INVALID_DEVICE_TYPE;
   XM7_DebugLog(XM7_LOG_DEBUG, "CL : Found %d processors.", ret_num_devices);

   using_device = processornum;
   if(using_device >= ret_num_devices) using_device = ret_num_devices - 1;
   if(using_device <= 0) using_device = 0;

   bCLEnableKhrGLShare = 0;

   for(i = 0; i < ret_num_devices; i++ ){

     extension_data[0] = '\0';
     GetDeviceName(extension_data, sizeof(extension_data), i);
     XM7_DebugLog(XM7_LOG_DEBUG, "CL : Processor #%d : Name = %s ", i, extension_data);

     extension_data[0] = '\0';
     clGetDeviceInfo(device_id[i], CL_DEVICE_TYPE,
		     sizeof(cl_ulong), &(device_type[i]), &llen);
     clGetDeviceInfo(device_id[i], CL_DEVICE_LOCAL_MEM_SIZE,
		     sizeof(cl_ulong), &(local_memsize[i]), &llen);
     GetDeviceType(extension_data, sizeof(extension_data), i);
     XM7_DebugLog(XM7_LOG_DEBUG, "CL : Processor #%d : TYPE = %s / Local memory size = %d bytes", i, extension_data, local_memsize[i]);

     extension_data[0] = '\0';
     clGetDeviceInfo(device_id[i], CL_DEVICE_EXTENSIONS,
		   1024, extension_data, &extension_len);
     XM7_DebugLog(XM7_LOG_DEBUG, "CL : Extension features(#%d):%s", i, extension_data);
     if(i == using_device) {
       if(strcasestr(extension_data, "cl_khr_gl_sharing") != NULL) {
	 if(GLinterop != 0) bCLEnableKhrGLShare = -1;
       } else {
	 bCLEnableKhrGLShare = 0;
       }
     }
   }
   
   XM7_DebugLog(XM7_LOG_DEBUG, "CL : Using device #%d", using_device);
   if(bCLEnableKhrGLShare != 0) { // This is only under X11. Must fix.
     XM7_DebugLog(XM7_LOG_DEBUG, "CL : GL Interoperability enabled.");
     properties[0] = CL_GL_CONTEXT_KHR;
     properties[1] = (cl_context_properties)glXGetCurrentContext();
     XM7_DebugLog(XM7_LOG_DEBUG, "CL : GL Context = %08x", glXGetCurrentContext());
     properties[2] = CL_GLX_DISPLAY_KHR;
     properties[3] = (cl_context_properties)glXGetCurrentDisplay();
     XM7_DebugLog(XM7_LOG_DEBUG, "CL : GL Display = %08x", glXGetCurrentDisplay());
     properties[4] = CL_CONTEXT_PLATFORM;
     properties[5] = (cl_context_properties)platform_id[platform_num];
     properties[6] = 0;
   } else {
     XM7_DebugLog(XM7_LOG_DEBUG, "CL : GL Interoperability disabled.");
     properties[0] = CL_CONTEXT_PLATFORM;
     properties[1] = (cl_context_properties)platform_id[platform_num];
     properties[2] = 0;
   }
//   if(device_id == NULL) return -1;
   
   context = clCreateContext(properties, 1, &device_id[using_device], cl_notify_log, NULL, &ret);
   XM7_DebugLog(XM7_LOG_DEBUG, "CL : Created context : STS = %d", ret);
   if(ret != CL_SUCCESS) return ret;
       
   command_queue = clCreateCommandQueue(context, device_id[using_device],
                                         CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &ret);
   XM7_DebugLog(XM7_LOG_DEBUG, "CL: Created command queue.");
   return ret;
}

static void CL_LogProgramExecute(cl_program program, void *userdata)
{
  char *logBuf;
  size_t length;
  cl_int r;
  cl_int n;
  cl_int num;
  cl_device_id *devid;
  class GLCLDraw *t = (class GLCLDraw *)userdata;

  logBuf = (char *)malloc(LOGSIZE * sizeof(char));
  if((logBuf == NULL) || (t == NULL))return;
  num = t->ret_num_devices;
  devid = t->device_id;
  //  printf("DBG: %08x %d\n", t, num);
  for(n = 0; n < num; n++) {
    logBuf[0] = '\0';
    r = clGetProgramBuildInfo(program, devid[n],  CL_PROGRAM_BUILD_LOG, 
			      LOGSIZE - 1, (void *)logBuf, &length);
    if((length > 0) && (length <= LOGSIZE)){
      logBuf[length] = '\0';
      if(strlen(logBuf) > 0) XM7_DebugLog(XM7_LOG_INFO, "CL :Build Log of Device #%d:%s", n, logBuf);
    }
  }
  free(logBuf);
  return;
}


cl_int GLCLDraw::BuildFromSource(const char *p)
{
    cl_int ret;
    size_t codeSize;
    char *logBuf;
    char compile_options[2048];
    cl_bool endian_little;
    compile_options[0] = '\0';
   
    codeSize = strlen(p);
    program = clCreateProgramWithSource(context, 1, (const char **)&p,
                                        (const size_t *)&codeSize, &ret);
    XM7_DebugLog(XM7_LOG_INFO, "CL: Build Result=%d", ret);
    if(ret < CL_SUCCESS) {
      return ret;
    }


    // Compile from source
    strncat(compile_options, "-cl-fast-relaxed-math ", sizeof(compile_options) - 1);
    if(clGetDeviceInfo(device_id[using_device], CL_DEVICE_ENDIAN_LITTLE,
		       sizeof(cl_bool), &endian_little, NULL) == CL_SUCCESS){
      if(endian_little == CL_TRUE) {
	strncat(compile_options, "-D_CL_KERNEL_LITTLE_ENDIAN=1 ", sizeof(compile_options) - 1);
      } else {
	strncat(compile_options, "-D_CL_KERNEL_LITTLE_ENDIAN=0 ", sizeof(compile_options) - 1); // Big endian
      }
    } else {
      strncat(compile_options, "-D_CL_KERNEL_LITTLE_ENDIAN=1 ", sizeof(compile_options) - 1); // Assume little endian
    }
    build_callback = CL_LogProgramExecute;
    ret = clBuildProgram(program, 1, &device_id[using_device], compile_options,
			 build_callback, (void *)this);
    XM7_DebugLog(XM7_LOG_INFO, "Compile Result=%d", ret);
    if(ret != CL_SUCCESS) {  // Printout error log.
      //      clReleaseProgram(program);
      return ret;
    }
    ret = clCreateKernelsInProgram(program, sizeof(kernels_array) / sizeof(cl_kernel),
				   kernels_array, &nkernels);
    if(ret < CL_SUCCESS) {
      XM7_DebugLog(XM7_LOG_INFO, "Unable to build CL kernel. Status=%d", ret);
    } else {
      char funcname[128];
      int i;
      size_t size;
      XM7_DebugLog(XM7_LOG_INFO, "Built %d CL kernel(s).", nkernels);
      for(i = 0; i < nkernels; i++) {
	funcname[0] = '\0';
	if(clGetKernelInfo(kernels_array[i], CL_KERNEL_FUNCTION_NAME,
			   sizeof(funcname) / sizeof(char) - 1, 
			   funcname, size) == CL_SUCCESS){
	  if((strncmp(funcname, "getvram8", strlen("getvram8")) == 0) && (kernel_8colors == NULL)) kernel_8colors = &kernels_array[i];
	  if((strncmp(funcname, "getvram4096", strlen("getvram4096")) == 0) && (kernel_4096colors == NULL)) kernel_4096colors = &kernels_array[i];
	  if((strncmp(funcname, "getvram256k", strlen("getvram256k")) == 0) && (kernel_256kcolors == NULL)) kernel_256kcolors = &kernels_array[i];
	  if((strncmp(funcname, "CreateTable", strlen("CreateTable")) == 0) && (kernel_table == NULL)) kernel_table = &kernels_array[i];
	  if((strncmp(funcname, "CopyVram", strlen("CopyVram")) == 0) && (kernel_copyvram == NULL)) kernel_copyvram = &kernels_array[i];
	}
      }
    }
   return ret;
}

Uint8 *GLCLDraw::GetBufPtr(Uint32 timeout)
{
  Uint32 t = timeout / 10;
  Uint32 i;
  BOOL flag = FALSE;
  if(timeout == 0) {
    AG_MutexLock(&mutex_buffer);
    return TransferBuffer;
  } else {
    for(i = 0; i < t; i++) {
      if(AG_MutexTryLock(&mutex_buffer) == 0) {
	flag = TRUE;
	break;
      }
      AG_Delay(10);
    }
    if(flag == FALSE) {
      t = timeout % 10;
      AG_Delay(t);
      if(AG_MutexTryLock(&mutex_buffer) == 0) flag = TRUE;
    }
    if(flag == FALSE) return NULL;
    return TransferBuffer;
  }
}

void GLCLDraw::ReleaseBufPtr(void)
{
  AG_MutexUnlock(&mutex_buffer);
}

Uint8 *GLCLDraw::MapTransferBuffer(int bmode)
{
  Uint8 *p = NULL;
  cl_int ret;
  switch(bmode)
  {
  case SCR_200LINE:
    p = clEnqueueMapBuffer(command_queue, inbuf[inbuf_bank], CL_FALSE, CL_MAP_WRITE_INVALIDATE_REGION,
			 0, 0x4000 * 3,
			 0, NULL, &event_uploadvram[0], &ret);
    break;
  case SCR_400LINE:
    p = clEnqueueMapBuffer(command_queue, inbuf[inbuf_bank], CL_FALSE, CL_MAP_WRITE_INVALIDATE_REGION,
			 0, 0x8000 * 3,
			 0, NULL, &event_uploadvram[0], &ret);
    break;
  case SCR_4096:
    p = clEnqueueMapBuffer(command_queue, inbuf[inbuf_bank], CL_FALSE, CL_MAP_WRITE_INVALIDATE_REGION,
			 0, 0x2000 * 12,
			 0, NULL, &event_uploadvram[0], &ret);
    break;
  case SCR_262144:
    p = clEnqueueMapBuffer(command_queue, inbuf[inbuf_bank], CL_FALSE, CL_MAP_WRITE_INVALIDATE_REGION,
			 0, 0x2000 * 18,
			 0, NULL, &event_uploadvram[0], &ret);
    break;
  }
  if(ret < CL_SUCCESS) p = NULL;
  return p;
}

cl_int GLCLDraw::UnMapTransferBuffer(Uint8 *p)
{
  cl_int ret;
  if(p == NULL) return CL_INVALID_MEM_OBJECT;
  ret = clEnqueueUnmapMemObject(command_queue, inbuf[inbuf_bank],
				 p, 0, NULL, &event_uploadvram[1]);
  return ret;
}

void GLCLDraw::AddPalette(int line, Uint8 mpage, BOOL analog)
{
   struct palettebuf_t *p;
   
   if(palettebuf == NULL) return;
   p = palettebuf;
   AG_MutexLock(&mutex_palette);
   if(analog) {
      int i;
      Uint32 lines;
      if(line < 0) line = 0;
      if(line > 199) line = 199;
      lines = p->alines_h * 256 + p->alines_l;
      if((lastline != line) || (lines == 1)) {
	 lastline = line;
	 lines++;
	 if(lines > 199) {
	    AG_MutexUnlock(&mutex_palette);
	    return;
	 }
	 
      }
      //printf("AddPalette %d\n", lines);
      p->alines_h = lines / 256;
      p->alines_l = lines % 256;
      p->atbls[lines - 1].line_h = line / 256;  
      p->atbls[lines - 1].line_l = line % 256;
      p->atbls[lines - 1].mpage = mpage;
      for(i = 0; i < 4096; i++) {
	 p->atbls[lines - 1].r_4096[i] = apalet_r[i];
	 p->atbls[lines - 1].g_4096[i] = apalet_g[i];
	 p->atbls[lines - 1].b_4096[i] = apalet_b[i];
      }
   } else {
      int i;
      Uint32 lines;
      int h = 199;
      
      if(bMode == SCR_400LINE) h = 399;
      if(line < 0) line = 0;
      if(line > h) line = h;
      lines = p->dlines_h * 256 + p->dlines_l;
      if((lastline != line) || (lines == 1)) {
	 lines++;
	 lastline = line;
	 if(lines > h) {
	    AG_MutexUnlock(&mutex_palette);
	    return;
	 }
	 
      }
      p->dlines_h = lines / 256;
      p->dlines_l = lines % 256;
      p->dtbls[lines - 1].line_h = line / 256;  
      p->dtbls[lines - 1].line_l = line % 256;
      p->dtbls[lines - 1].mpage = mpage;
      for(i = 0; i < 7; i++) {
	 p->dtbls[lines - 1].tbl[i] = ttl_palet[i];
      }
   }
   AG_MutexUnlock(&mutex_palette);

}

void GLCLDraw::ResetPalette(void)
{
   struct palettebuf_t *pold, *pnew;
   int newline;
   int endline;
   cl_int r;
   cl_event ev_unmap, ev_map;
   int i;
   

 //  CopyPalette();
   AG_MutexLock(&mutex_palette);
   //   pold = palettebuf;
   pnew = palettebuf;
   lastline = 0;
   if(pnew != NULL) {
	palettebuf = pnew;
	     {
		pnew->alines_h = 0;
		pnew->alines_l = 1;
		pnew->atbls[0].line_h = 0;
		pnew->atbls[0].line_l = 0;
		pnew->atbls[0].mpage = multi_page;
		for(i = 0; i < 4096; i++) {
		   pnew->atbls[0].r_4096[i] = apalet_r[i];
		   pnew->atbls[0].g_4096[i] = apalet_g[i];
		   pnew->atbls[0].b_4096[i] = apalet_b[i];
		}
	     }
	     {
		pnew->dlines_h = 0;
		pnew->dlines_l = 1;
		pnew->dtbls[0].line_h = 0;
		pnew->dtbls[0].line_l = 0;
		pnew->dtbls[0].mpage = multi_page;
		for(i = 0; i < 8; i++) pnew->dtbls[0].tbl[i] = ttl_palet[i];
	     }
   }
   AG_MutexUnlock(&mutex_palette);
//   clFinish(command_queue);
//   clFlush(command_queue);
}

void GLCLDraw::CopyPalette(void)
{
   struct palettebuf_t *pold, *pnew;
   int newline;
   int endline;
   int alines;
   int dlines;
   cl_int r;
   cl_event ev_unmap, ev_map;
   int i;
   
   AG_MutexLock(&mutex_palette);

   pold = palettebuf;
   newline = palette_bank + 1;
   if(newline >= 2) newline = 0;
   
   pnew = clEnqueueMapBuffer(command_queue, palette_buf[newline], CL_TRUE, CL_MAP_READ | CL_MAP_WRITE,
			     0, (size_t)sizeof(struct palettebuf_t),
			     0, NULL, &ev_map, &r);
   if(r < CL_SUCCESS) {
      AG_MutexUnlock(&mutex_palette);
      return;
   }
   alines = pold->alines_h * 256 + pold->alines_l;
   dlines = pold->dlines_h * 256 + pold->dlines_l;
   if(alines < 0)   alines = 0;
   if(alines > 199) alines = 199;
   if(dlines < 0)   dlines = 0;
   if(dlines > 399) dlines = 399;

   if((pold != NULL) && (pnew != NULL)) {
     memcpy(pnew, pold, sizeof(Uint8) * 4 + sizeof(struct apalettetbl_t) * alines); // Copy Lines + Analog Palette
     memcpy(&(pnew->dtbls[0]), &(pold->dtbls[0]), sizeof(struct dpalettetbl_t) * dlines); // Copy Digital Palette
     palettebuf = pnew;
     
     clEnqueueUnmapMemObject(command_queue, palette_buf[palette_bank], 
			     pold, 1, &ev_map,
			     &ev_unmap);
     palette_bank_old = palette_bank;
     palette_bank = newline;
   }
   
   AG_MutexUnlock(&mutex_palette);
   //clFinish(command_queue);
   clFlush(command_queue);
}

	   
   


cl_int GLCLDraw::GetVram(int bmode)
{
   cl_int ret = 0;
   cl_int r;
   cl_kernel *kernel = NULL;
   int w = 0;
   int h = 0;
   Uint8 *pr,*pg,*pb;
   size_t lws[] = {10}; // local jobs.
   size_t gws[] = {nCLGlobalWorkThreads}; // Parallel jobs.
   size_t *goff = NULL;
   int mpage = multi_page;
   int dummy = 0;
   int vpage;
   int crtflag = crt_flag;
   int bank;
   BOOL flag = FALSE;
   int i;
   cl_float4 bright;
   cl_event copy_event;

   bright.s[0] = fBrightR; // R
   bright.s[1] = fBrightG; // G
   bright.s[2] = fBrightB; // B
   bright.s[3] = 1.0; // A
 
   
   //if(inbuf == NULL) return -1;
   if(outbuf == NULL) return -1;
   //if(TransferBuffer == NULL) return -1;
   /*
    * Swap Buffer
    */
   {
     size_t transfer_size = 0;
     bank = inbuf_bank;
     Uint8 *p;
     p = GetBufPtr(0); // Maybe okay?
     for(i = 0; i < 400 ; i++) {
       if(bDrawLine[i]) flag = TRUE;
       bDrawLine[i] = FALSE;
    }
    if(flag) {
       ret = UnMapTransferBuffer(p);
       if(ret < CL_SUCCESS) {
	 ReleaseBufPtr();
	 return ret;
       }
     }
     switch(bmode){
     case SCR_200LINE:
       transfer_size = 0x4000 * 3;
       break;
     case SCR_400LINE:
       transfer_size = 0x8000 * 3;
       break;
     case SCR_4096:
       transfer_size = 0x2000 * 12;
       break;
     case SCR_262144:
       transfer_size = 0x2000 * 18;
       break;
     }
     if((flag != FALSE) && (transfer_size > 0)){
       inbuf_bank++;
       if(inbuf_bank >= 2) inbuf_bank = 0;
       if(kernel_copyvram != NULL) {
	 size_t lws_copy[] = {1};
	 size_t gws_copy[] = {gws[0]};
	      
	 cl_int size = transfer_size;
	 ret |= clSetKernelArg(*kernel_copyvram, 0, sizeof(cl_mem), (void *)&(inbuf[inbuf_bank]));
	 ret |= clSetKernelArg(*kernel_copyvram, 1, sizeof(cl_mem), (void *)&(inbuf[bank]));
	 ret |= clSetKernelArg(*kernel_copyvram, 2, sizeof(cl_int), &size);
	 ret |= clSetKernelArg(*kernel_copyvram, 3, sizeof(cl_int), &bCLSparse);
	 if(bCLSparse) {
	    ret = clEnqueueNDRangeKernel(command_queue, *kernel_copyvram, 1, 
					 goff, gws_copy, lws_copy, 
					 0, NULL,  &copy_event);
	 } else {
	    ret = clEnqueueTask(command_queue,
				*kernel_copyvram, 0, NULL, &copy_event);
	 }
      } else {
       ret = clEnqueueCopyBuffer(command_queue, inbuf[bank], inbuf[inbuf_bank], 0,
				 0, transfer_size, 0, NULL,
			       &copy_event);
      }
      clFinish(command_queue);
      TransferBuffer = MapTransferBuffer(bmode);
     }
     ReleaseBufPtr();
     if(TransferBuffer == NULL) return CL_MEM_OBJECT_ALLOCATION_FAILURE;
   }
   if((flag) || bPaletFlag || SDLDrawFlag.APaletteChanged || SDLDrawFlag.DPaletteChanged) {
   kernel = NULL;
   LockVram();
   SDLDrawFlag.APaletteChanged = FALSE;
   SDLDrawFlag.DPaletteChanged = FALSE;
   SDLDrawFlag.Drawn = FALSE;
   bPaletFlag = FALSE;
   UnlockVram();
   CopyPalette();
   switch(bmode) {
    case SCR_400LINE:
    case SCR_200LINE:
      w = 640;
      h = 200;
      if(bmode == SCR_400LINE) h = 400;
      vpage = (~(multi_page >> 4)) & 0x07;
      //gws[0] = h;
      if(kernel_8colors != NULL) kernel = kernel_8colors;
      if(kernel != NULL) {
	 ret |= clSetKernelArg(*kernel, 0, sizeof(cl_mem), (void *)&(inbuf[bank]));
	 ret |= clSetKernelArg(*kernel, 1, sizeof(int),    (void *)&w);
	 ret |= clSetKernelArg(*kernel, 2, sizeof(int), (void *)&h);
	 ret |= clSetKernelArg(*kernel, 3, sizeof(cl_mem), (void *)&outbuf);
	 ret |= clSetKernelArg(*kernel, 4, sizeof(cl_mem), (void *)&palette_buf[palette_bank_old]);
	 ret |= clSetKernelArg(*kernel, 5, sizeof(cl_mem), (void *)&table);
	 ret |= clSetKernelArg(*kernel, 6, sizeof(int), (void *)&bCLSparse);
	 ret |= clSetKernelArg(*kernel, 7, sizeof(int), (void *)&crtflag);
	 ret |= clSetKernelArg(*kernel, 8, sizeof(cl_float4), (void *)&bright);
	 clFlush(command_queue);
      }
      break;
    case SCR_262144:// Windowはなし
      w = 320;
      h = 200;
      //gws[0] = h;

      //      if(kernel == NULL) kernel = clCreateKernel(program, "getvram256k", &ret);
      if(kernel_256kcolors != NULL) kernel = kernel_256kcolors;
      if(kernel != NULL) {
      /*
       * Below transfer is dummy.
       */
	 ret |= clSetKernelArg(*kernel, 0, sizeof(cl_mem),  (void *)&(inbuf[bank]));
	 ret |= clSetKernelArg(*kernel, 1, sizeof(cl_int),  (void *)&w);
	 ret |= clSetKernelArg(*kernel, 2, sizeof(cl_int),  (void *)&h);
	 ret |= clSetKernelArg(*kernel, 3, sizeof(cl_mem),  (void *)&outbuf);
	 ret |= clSetKernelArg(*kernel, 4, sizeof(cl_mem),  (void *)&table);
	 ret |= clSetKernelArg(*kernel, 5, sizeof(cl_uint), (void *)&mpage);
	 ret |= clSetKernelArg(*kernel, 6, sizeof(cl_int),  (void *)&bCLSparse);
	 ret |= clSetKernelArg(*kernel, 7, sizeof(cl_int),  (void *)&crtflag);
	 ret |= clSetKernelArg(*kernel, 8, sizeof(cl_float4), (void *)&bright);
	 clFlush(command_queue);
      }
      break;
    case SCR_4096:
      w = 320;
      h = 200;
      //gws[0] = h;
      //if(kernel == NULL) kernel = clCreateKernel(program, "getvram4096", &ret);
      if(kernel_4096colors != NULL) kernel = kernel_4096colors;
      if(kernel != NULL) {
	 
	 ret |= clSetKernelArg(*kernel, 0, sizeof(cl_mem),  (void *)&(inbuf[bank]));
	 ret |= clSetKernelArg(*kernel, 1, sizeof(cl_int),  (void *)&w);
	 ret |= clSetKernelArg(*kernel, 2, sizeof(cl_int),  (void *)&h);
	 ret |= clSetKernelArg(*kernel, 3, sizeof(cl_mem),  (void *)&outbuf);
	 ret |= clSetKernelArg(*kernel, 4, sizeof(cl_mem),  (void *)&(palette_buf[palette_bank_old]));
	 ret |= clSetKernelArg(*kernel, 5, sizeof(cl_mem),  (void *)&table);
	 ret |= clSetKernelArg(*kernel, 6, sizeof(cl_int),  (void *)&bCLSparse);
	 ret |= clSetKernelArg(*kernel, 7, sizeof(cl_int),  (void *)&crtflag);
	 ret |= clSetKernelArg(*kernel, 8, sizeof(cl_float4), (void *)&bright);
      //clFinish(command_queue);
	 clFlush(command_queue);
      }
      break;
   }
   w2 = w;
   h2 = h;
   //CopyPalette();
   if(bCLEnableKhrGLShare != 0) {
     glFlush();
     ret |= clEnqueueAcquireGLObjects (command_queue,
				  1, (cl_mem *)&outbuf,
				  0, NULL, &event_copytotexture);
     if(kernel != NULL) {
       if(bCLSparse) {
	 ret = clEnqueueNDRangeKernel(command_queue, *kernel, 1, 
				      goff, gws, lws, 
				      1, &event_copytotexture,  &event_exec);
       } else {
	 ret = clEnqueueTask(command_queue,
			     *kernel, 1, &event_copytotexture, &event_exec);
       }
     }
      
     ret |= clEnqueueReleaseGLObjects (command_queue,
				  1, (cl_mem *)&outbuf,
				  1, &event_exec, &event_release);
     clFinish(command_queue);
//     glFinish();
   } else {
     if(kernel != NULL) {
       if(bCLSparse) {
	 ret = clEnqueueNDRangeKernel(command_queue, *kernel, 1, 
				      goff, gws, lws, 
				      1, &event_uploadvram[2],  &event_exec);
       } else {
	 ret = clEnqueueTask(command_queue,
			     *kernel, 1, &event_uploadvram[2], &event_exec);
       }
     }
   }
   }
   
   return ret;
}


cl_int GLCLDraw::SetupTable(void)
{
   cl_int r = CL_INVALID_KERNEL;
   cl_uint pages;
   cl_event tbl_ev;
   pages = 6;

   if(kernel_table != NULL) {
     r = 0;
      r |= clSetKernelArg(*kernel_table, 0, sizeof(cl_mem),     (void *)&table);
      r |= clSetKernelArg(*kernel_table, 1, sizeof(cl_uint),    (void *)&pages);
      r |= clEnqueueTask(command_queue,
			     *kernel_table, 0, NULL, NULL);
      clFinish(command_queue);
    }
   return r;
}


cl_int GLCLDraw::SetupBuffer(GLuint *texid)
{
   cl_int ret = 0;
   cl_int r = 0;
   cl_event ev;
   unsigned int size = 640 * 400 * sizeof(cl_uchar4);
   int i;

   inbuf_bank = 0;
   for(i = 0; i < 2; i++) {
     inbuf[i] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | 0, // Reduce HOST-CPU usage.
                            (size_t)(0x2000 * 18 * sizeof(Uint8)), NULL, &r);
     ret |= r;
     if(r == CL_SUCCESS){
       cl_int r2;
       cl_event cl_event_map;
       cl_event cl_event_unmap;
       Uint8 *p;

       p = clEnqueueMapBuffer(command_queue, inbuf[i], CL_TRUE, CL_MAP_WRITE,
			      0, (size_t)(0x2000 * 18 * sizeof(Uint8)),
			 0, NULL, &cl_event_map, &r2);
       if((r2 >= CL_SUCCESS) && (p != NULL)) {
       memset(p, 0x00, (size_t)(0x2000 * 18 * sizeof(Uint8)));
       clEnqueueUnmapMemObject(command_queue, inbuf[i], 
				 p, 1, &cl_event_map,
				 &cl_event_unmap);
	clFinish(command_queue);
       }
     }
     XM7_DebugLog(XM7_LOG_INFO, "CL: Alloc STS: inbuf[%d] : %d", i, r);
     palette_buf[i] = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR | 0, // Reduce HOST-CPU usage.
                            (size_t)sizeof(struct palettebuf_t), NULL, &r);
     ret = r;
     if(r == CL_SUCCESS){
       cl_int r2;
       cl_event cl_event_map;
       cl_event cl_event_unmap;
       Uint8 *p;
       p = clEnqueueMapBuffer(command_queue, palette_buf[i], CL_TRUE, CL_MAP_WRITE,
			      0, (size_t)sizeof(struct palettebuf_t),
	                      0, NULL, &cl_event_map, &r2);
       if((r2 >= CL_SUCCESS) && (p != NULL)) {
	  memset(p, 0x00, (size_t)sizeof(struct palettebuf_t));
	  clEnqueueUnmapMemObject(command_queue, palette_buf[i], 
				  p, 1, &cl_event_map,
				  &cl_event_unmap);
	  clFinish(command_queue);
       }
     }
     XM7_DebugLog(XM7_LOG_INFO, "CL: Alloc STS: palette_buf[%d] : %d", i, r);
   }
   TransferBuffer = MapTransferBuffer(SCR_262144);
   palettebuf = clEnqueueMapBuffer(command_queue, palette_buf[0], CL_TRUE, CL_MAP_WRITE,
			      0, (size_t)sizeof(struct palettebuf_t),
	                      0, NULL, &ev, &r);
   ResetPalette();
   table = clCreateBuffer(context, CL_MEM_READ_WRITE | 0,
 		  (size_t)(0x100 * 8 * 6 * sizeof(cl_uint)), NULL, &r);
   ret |= r;
   XM7_DebugLog(XM7_LOG_INFO, "CL: Alloc STS: table : %d", r);

   // Texture直接からPBO使用に変更 20121102
   if((bCLEnableKhrGLShare != 0) && (bGL_PIXEL_UNPACK_BUFFER_BINDING != FALSE)){
       glGenBuffers(1, &pbo);
       if(pbo <= 0) {
	 bCLEnableKhrGLShare = FALSE;
	 goto _fallback;
       }

       glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
       //glBufferData(GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
       glBufferData(GL_PIXEL_UNPACK_BUFFER, size, NULL, GL_STREAM_DRAW);
       //    XM7_DebugLog(XM7_LOG_DEBUG, "CL: PBO=%08x Size=%d context=%08x", pbo, size, context);
       outbuf = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY | 0, 
				     pbo, &r);
       if(r != GL_NO_ERROR) {
	 glDeleteBuffers(1, &pbo);
//	 pbo = 0;
	 bCLEnableKhrGLShare = FALSE;
	 goto _fallback;
       }
       glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
       ret |= r;
       XM7_DebugLog(XM7_LOG_INFO, "CL: Alloc STS: outbuf (GLCL Interop): %d", r);
       return ret;
   }
 _fallback:
   outbuf = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
			   (size_t)(640 * 400 * sizeof(Uint32)), NULL, &r);
   ret |= r;
   XM7_DebugLog(XM7_LOG_INFO, "CL: Alloc STS: outbuf (CL): %d", r);
   return ret;
}

GLuint GLCLDraw::GetPbo(void)
{
   return pbo;
}
