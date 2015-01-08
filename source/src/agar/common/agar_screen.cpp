/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 screen ]
*/

#include "emu.h"
#include "vm/vm.h"
#include "agar_main.h"
#include "agar_logger.h"


#define RESULT_SUCCESS	1
#define RESULT_FULL	2
#define RESULT_ERROR	3

#if defined(_USE_AGAR) || defined(_USE_SDL)
void *rec_video_thread(void *lpx);
#else
unsigned __stdcall rec_video_thread(void *lpx);
#endif
#ifdef USE_CRT_FILTER
static uint8 r0[2048], g0[2048], b0[2048], t0[2048];
static uint8 r1[2048], g1[2048], b1[2048];
#endif

extern "C" {
   int bFullScan = 0;
}


void EMU::initialize_screen()
{
	screen_width = SCREEN_WIDTH;
	screen_height = SCREEN_HEIGHT;
	screen_width_aspect = SCREEN_WIDTH_ASPECT;
	screen_height_aspect = SCREEN_HEIGHT_ASPECT;
	window_width = WINDOW_WIDTH;
	window_height = WINDOW_HEIGHT;
	screen_size_changed = true;
	
	source_width = source_height = -1;
	source_width_aspect = source_height_aspect = -1;
	stretch_pow_x = stretch_pow_y = -1;
	stretch_screen = false;

	// Agar specific value
	render_to_GL = false;
	render_with_OpenCL = false;
	render_to_SDLFB = false;
	use_GL = false;
	use_SDLFB = false;
	wait_vsync = false;
        // *nix only, need to change on WIndows.
        if(posix_memalign(&pPseudoVram, 64, sizeof(Uint32) * SCREEN_WIDTH * SCREEN_HEIGHT) != 0){
	   pPseudoVram = NULL;
	}
        {
	   int i;
	   for(i = 0; i < SCREEN_HEIGHT; i++) {
		bDrawLine[i] = false;
	   }
	}
   
	

	//if(AG_UsingGL()) {
	//  render_to_GL = true;
	//  use_GL = true;
	//} else {
	  render_to_SDLFB = true;
	  use_SDLFB = true;
	  //}
	  single_window = false;
	now_rec_video = false;
	
	// initialize update flags
	first_draw_screen = false;
	first_invalidate = self_invalidate = false;
	
#ifdef USE_CRT_FILTER
	// initialize crtc filter
	memset(r1, 0, sizeof(r1));
	memset(g1, 0, sizeof(g1));
	memset(b1, 0, sizeof(b1));
#endif
}


void EMU::release_screen()
{
        if(pPseudoVram != NULL) free(pPseudoVram);
         pPseudoVram = NULL;
	// stop video recording
	//stop_rec_video();
}



int EMU::get_window_width(int mode)
{
#ifdef USE_SCREEN_ROTATE
	if(config.monitor_type) {
		return window_height + screen_height_aspect * mode;
	}
#endif
	return window_width + screen_width_aspect * mode;
}

int EMU::get_window_height(int mode)
{
#ifdef USE_SCREEN_ROTATE
	if(config.monitor_type) {
		return window_width + screen_width_aspect * mode;
	}
#endif
	return window_height + screen_height_aspect * mode;
}

void EMU::set_display_size(int width, int height, bool window_mode)
{
   bool display_size_changed = false;
   bool stretch_changed = false;
   int prev_stretched_width = stretched_width;
   int prev_stretched_height = stretched_height;
   AGAR_DebugLog(AGAR_LOG_DEBUG, "Set display size");
   AGAR_DebugLog(AGAR_LOG_DEBUG, "       to %d x %d", width, height);
   
   if(width != -1 && (display_width != width || display_height != height)) {
      display_width = width;
      display_height = height;
      display_size_changed = stretch_changed = true;
   }
//   if(use_d3d9 != config.use_d3d9) {
   //if(!(use_d3d9 = config.use_d3d9)) {
   //   release_d3d9();
   //}
   //display_size_changed = stretch_changed = true;
//}

   if(main_window_handle != NULL) {
	AG_WindowSetMinSize(main_window_handle, display_width, display_height + 40); // Temporally
   }

#ifdef USE_SCREEN_ROTATE
	if(config.monitor_type) {
		
		stretch_changed |= (source_width != screen_height);
		stretch_changed |= (source_height != screen_width);
		stretch_changed |= (source_width_aspect != screen_height_aspect);
		stretch_changed |= (source_height_aspect != screen_width_aspect);
		
		source_width = screen_height;
		source_height = screen_width;
		source_width_aspect = screen_height_aspect;
		source_height_aspect = screen_width_aspect;
		
//		render_to_d3d9Buffer = false;
	} else {
#endif
		
		stretch_changed |= (source_width != screen_width);
		stretch_changed |= (source_height != screen_height);
		stretch_changed |= (source_width_aspect != screen_width_aspect);
		stretch_changed |= (source_height_aspect != screen_height_aspect);
		
		source_width = screen_width;
		source_height = screen_height;
		source_width_aspect = screen_width_aspect;
		source_height_aspect = screen_height_aspect;
#ifdef USE_SCREEN_ROTATE
	}
#endif

   if(config.stretch_type == 1) {
		// fit to full screen (aspect)
      stretched_width = (display_height * source_width_aspect) / source_height_aspect;
      stretched_height = display_height;
      if(stretched_width > display_width) {
		   stretched_width = display_width;
		   stretched_height = (display_width * source_height_aspect) / source_width_aspect;
      }
   } else if(config.stretch_type == 2) {
		// fit to full screen (fill)
		stretched_width = display_width;
		stretched_height = display_height;
   } else {
      // dot by dot
      int tmp_pow_x = display_width / source_width_aspect;
      int tmp_pow_y = display_height / source_height_aspect;
      int tmp_pow = 1;
      if(tmp_pow_y >= tmp_pow_x && tmp_pow_x > 1) {
	 tmp_pow = tmp_pow_x;
      } else if(tmp_pow_x >= tmp_pow_y && tmp_pow_y > 1) {
	 tmp_pow = tmp_pow_y;
      }
      stretched_width = source_width_aspect * tmp_pow;
      stretched_height = source_height_aspect * tmp_pow;
   }
   screen_dest_x = (display_width - stretched_width) / 2;
   screen_dest_y = (display_height - stretched_height) / 2;
   
   stretch_changed |= (prev_stretched_width != stretched_width);
   stretch_changed |= (prev_stretched_height != stretched_height);
   
   int new_pow_x = 1, new_pow_y = 1;
   while(stretched_width > source_width * new_pow_x) {
      new_pow_x++;
   }
   while(stretched_height > source_height * new_pow_y) {
      new_pow_y++;
   }
//	if(!use_d3d9 && new_pow_x > 1 && new_pow_y > 1) {
//		// support high quality stretch only for x1 window size in gdi mode
//		new_pow_x = new_pow_y = 1;
//	}
   if(stretch_pow_x != new_pow_x || stretch_pow_y != new_pow_y) {
      stretch_pow_x = new_pow_x;
      stretch_pow_y = new_pow_y;
      stretch_changed = true;
   }
//   if(stretch_pow_x != 1 || stretch_pow_y != 1) {
//      render_to_d3d9Buffer = false;
//   }
   
   if(instance_handle) {
      AG_SizeAlloc s_alloc;
      s_alloc.x = 0;
      s_alloc.y = 0;
      s_alloc.w = stretched_width;
      s_alloc.h = stretched_height;
      AG_WidgetSizeAlloc(instance_handle, &s_alloc);
      AG_WidgetSetSize(instance_handle, stretched_width, stretched_height);
   }
   
   first_draw_screen = false;
   first_invalidate = true;
   screen_size_changed = false;

}

void EMU::change_screen_size(int sw, int sh, int swa, int sha, int ww, int wh)
{
   AGAR_DebugLog(AGAR_LOG_DEBUG, "Change Screen Width");
   AGAR_DebugLog(AGAR_LOG_DEBUG, "       From %d x %d", screen_width, screen_height);
  // virtual machine changes the screen size
	if(screen_width != sw || screen_height != sh) {
		screen_width = sw;
		screen_height = sh;
		screen_width_aspect = (swa != -1) ? swa : sw;
		screen_height_aspect = (sha != -1) ? sha : sh;
		window_width = ww;
		window_height = wh;
		screen_size_changed = true;
		
		// re-create dib sections
//		HDC hdc = GetDC(main_window_handle);
//		release_dib_section(hdcDib, hBmp, hOldBmp, lpBuf);
//		create_dib_section(hdc, screen_width, screen_height, &hdcDib, &hBmp, &hOldBmp, &lpBuf, &lpBmp, &lpDib);
#ifdef USE_SCREEN_ROTATE
//		release_dib_section(hdcDibRotate, hBmpRotate, hOldBmpRotate, lpBufRotate);
//		create_dib_section(hdc, screen_height, screen_width, &hdcDibRotate, &hBmpRotate, &hOldBmpRotate, &lpBufRotate, &lpBmpRotate, &lpDibRotate);
#endif
//		ReleaseDC(main_window_handle, hdc);
		
		// stop recording
		if(now_rec_video) {
			stop_rec_video();
			stop_rec_sound();
		}
		
		// change the window size
		//AG_PushEvent(main_window_handle, WM_RESIZE, 0L, 0L);
	}
   AGAR_DebugLog(AGAR_LOG_DEBUG, "       To   %d x %d", screen_width, screen_height);
   AGAR_DebugLog(AGAR_LOG_DEBUG, "Window Size:%d x %d", window_width, window_height);
   if(main_window_handle) AG_WindowSetMinSize(main_window_handle, window_width, window_height);

}

int EMU::draw_screen()
{
#if 0
        // don't draw screen before new screen size is applied to buffers
	if(screen_size_changed) {
		return 0;
	}
	
	// check avi file recording timing
	if(now_rec_video && rec_video_run_frames <= 0) {
		return 0;
	}
#endif	
	// lock offscreen surface
	
	// draw screen
	vm->draw_screen();
	
	// screen size was changed in vm->draw_screen()
	if(screen_size_changed) {
		// unlock offscreen surface
		return 0;
	}
#if 0	
#ifdef USE_SCREEN_ROTATE
	// rotate screen
	if(config.monitor_type) {
	}
#endif	
	
	// stretch screen
	if(stretch_screen) {
		scrntype* src = lpBmpSource + source_width * (source_height - 1);
		scrntype* out = lpBmpStretch1 + source_width * stretch_pow_x * (source_height * stretch_pow_y - 1);
		int data_len = source_width * stretch_pow_x;
#ifdef USE_CRT_FILTER
		#define _3_8(v) (((((v) * 3) >> 3) * 180) >> 8)
		#define _5_8(v) (((((v) * 3) >> 3) * 180) >> 8)
		#define _8_8(v) (((v) * 180) >> 8)
		
		if(config.crt_filter && stretch_pow_x == 3 && stretch_pow_y == 3) {
			r1[0] = g1[0] = b1[0] = r1[source_width + 1] = g1[source_width + 1] = b1[source_width + 1] = 0;
			
			if(!screen_skip_line) {
				for(int y = 0; y < source_height; y++) {
					for(int x = 1; x <= source_width; x++) {
						uint32 c = src[x - 1];
						t0[x] = (c >> 24) & 0xff;
						r0[x] = (c >> 16) & 0xff;
						g0[x] = (c >>  8) & 0xff;
						b0[x] = (c      ) & 0xff;
						r1[x] = (c >> 19) & 0x1f;
						g1[x] = (c >> 11) & 0x1f;
						b1[x] = (c >>  3) & 0x1f;
					}
					scrntype* out1 = out;
					out -= data_len;
					scrntype* out2 = out;
					out -= data_len;
					scrntype* out3 = out;
					out -= data_len;
					for(int x = 1, xx = 0; x <= source_width; x++, xx += 3) {
						uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
						uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
						uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
						out1[xx    ] = out2[xx    ] = (32 + _8_8(r)) << 16;
						out1[xx + 1] = out2[xx + 1] = (32 + _8_8(g)) << 8;
						out1[xx + 2] = out2[xx + 2] = (32 + _8_8(b));
						if(t0[x]) {
							out3[xx    ] = (32 + _8_8(r)) << 16;
							out3[xx + 1] = (32 + _8_8(g)) << 8;
							out3[xx + 2] = (32 + _8_8(b));
						} else {
							out3[xx    ] = (32 + _5_8(r)) << 16;
							out3[xx + 1] = (32 + _5_8(g)) << 8;
							out3[xx + 2] = (32 + _5_8(b));
						}
					}
					src -= source_width;
				}
			} else {
				for(int y = 0; y < source_height; y += 2) {
					for(int x = 1; x <= source_width; x++) {
						uint32 c = src[x - 1];
						t0[x] = (c >> 24) & 0xff;
						r0[x] = (c >> 16) & 0xff;
						g0[x] = (c >>  8) & 0xff;
						b0[x] = (c      ) & 0xff;
						r1[x] = (c >> 20) & 0x0f;
						g1[x] = (c >> 12) & 0x0f;
						b1[x] = (c >>  4) & 0x0f;
					}
					scrntype* out1 = out;
					out -= data_len;
					scrntype* out2 = out;
					out -= data_len;
					scrntype* out3 = out;
					out -= data_len;
					scrntype* out4 = out;
					out -= data_len;
					scrntype* out5 = out;
					out -= data_len;
					scrntype* out6 = out;
					out -= data_len;
					for(int x = 1, xx = 0; x <= source_width; x++, xx += 3) {
						uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
						uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
						uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
						out1[xx    ] = out2[xx    ] = out3[xx    ] = out4[xx    ] = (32 + _8_8(r)) << 16;
						out1[xx + 1] = out2[xx + 1] = out3[xx + 1] = out4[xx + 1] = (32 + _8_8(g)) << 8;
						out1[xx + 2] = out2[xx + 2] = out3[xx + 2] = out4[xx + 2] = (32 + _8_8(b));
						if(t0[x]) {
							out5[xx    ] = out6[xx    ] = (32 + _8_8(r)) << 16;
							out5[xx + 1] = out6[xx + 1] = (32 + _8_8(g)) << 8;
							out5[xx + 2] = out6[xx + 2] = (32 + _8_8(b));
						} else {
							out5[xx    ] = out6[xx    ] = (32 + _5_8(r)) << 16;
							out5[xx + 1] = out6[xx + 1] = (32 + _5_8(g)) << 8;
							out5[xx + 2] = out6[xx + 2] = (32 + _5_8(b));
						}
					}
					src -= source_width * 2;
				}
			}
		} else if(config.crt_filter && stretch_pow_x == 2 && stretch_pow_y == 2) {
			if(!screen_skip_line) {
				for(int y = 0; y < source_height; y++) {
					for(int x = 1; x <= source_width; x++) {
						uint32 c = src[x - 1];
						t0[x] = (c >> 24) & 0xff;
						r0[x] = (c >> 16) & 0xff;
						g0[x] = (c >>  8) & 0xff;
						b0[x] = (c      ) & 0xff;
						r1[x] = (c >> 19) & 0x1f;
						g1[x] = (c >> 11) & 0x1f;
						b1[x] = (c >>  3) & 0x1f;
					}
					scrntype* out1 = out;
					out -= data_len;
					scrntype* out2 = out;
					out -= data_len;
					for(int x = 1, xx = 0; x <= source_width; x++, xx += 2) {
						uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
						uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
						uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
						out1[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
						out1[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
						if(t0[x]) {
							out2[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
							out2[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
						} else {
							out2[xx    ] = RGB_COLOR(32 + _3_8(r), 32 + _3_8(g), 32 + _3_8(b));
							out2[xx + 1] = RGB_COLOR(16 + _3_8(r), 16 + _3_8(g), 16 + _3_8(b));
						}
					}
					src -= source_width;
				}
			} else {
				for(int y = 0; y < source_height; y += 2) {
					for(int x = 1; x <= source_width; x++) {
						uint32 c = src[x - 1];
						t0[x] = (c >> 24) & 0xff;
						r0[x] = (c >> 16) & 0xff;
						g0[x] = (c >>  8) & 0xff;
						b0[x] = (c      ) & 0xff;
						r1[x] = (c >> 19) & 0x1f;
						g1[x] = (c >> 11) & 0x1f;
						b1[x] = (c >>  3) & 0x1f;
					}
					scrntype* out1 = out;
					out -= data_len;
					scrntype* out2 = out;
					out -= data_len;
					scrntype* out3 = out;
					out -= data_len;
					scrntype* out4 = out;
					out -= data_len;
					for(int x = 1, xx = 0; x <= source_width; x++, xx += 2) {
						uint32 r = r1[x - 1] + r0[x] + r1[x + 1];
						uint32 g = g1[x - 1] + g0[x] + g1[x + 1];
						uint32 b = b1[x - 1] + b0[x] + b1[x + 1];
						out1[xx    ] = out2[xx    ] = out3[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
						out1[xx + 1] = out2[xx + 1] = out3[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
						if(t0[x]) {
							out4[xx    ] = RGB_COLOR(32 + _8_8(r), 32 + _8_8(g), 32 + _8_8(b));
							out4[xx + 1] = RGB_COLOR(16 + _5_8(r), 16 + _5_8(g), 16 + _5_8(b));
						} else {
							out4[xx    ] = RGB_COLOR(32 + _3_8(r), 32 + _3_8(g), 32 + _3_8(b));
							out4[xx + 1] = RGB_COLOR(16 + _3_8(r), 16 + _3_8(g), 16 + _3_8(b));
						}
					}
					src -= source_width * 2;
				}
			}
		} else
#endif
		for(int y = 0; y < source_height; y++) {
			if(stretch_pow_x != 1) {
				scrntype* out_tmp = out;
				for(int x = 0; x < source_width; x++) {
					scrntype c = src[x];
					for(int px = 0; px < stretch_pow_x; px++) {
						out_tmp[px] = c;
					}
					out_tmp += stretch_pow_x;
				}
			} else {
				// faster than memcpy()
				for(int x = 0; x < source_width; x++) {
					out[x] = src[x];
				}
			}
			if(stretch_pow_y != 1) {
				scrntype* src_tmp = out;
				for(int py = 1; py < stretch_pow_y; py++) {
					out -= data_len;
					// about 10% faster than memcpy()
					for(int x = 0; x < data_len; x++) {
						out[x] = src_tmp[x];
					}
				}
			}
			src -= source_width;
			out -= data_len;
		}
		if(!use_d3d9) {
			StretchBlt(hdcDibStretch2, 0, 0, stretched_width, stretched_height, hdcDibStretch1, 0, 0, source_width * stretch_pow_x, source_height * stretch_pow_y, SRCCOPY);
		}
	}
	first_draw_screen = true;
	
	// copy bitmap to d3d9 offscreen surface
	if(use_d3d9 && lpd3d9Buffer != NULL) {
		if(!(render_to_d3d9Buffer && !now_rec_video)) {
			scrntype *src = stretch_screen ? lpBmpStretch1 : lpBmpSource;
			src += source_width * stretch_pow_x * (source_height * stretch_pow_y - 1);
			scrntype *out = lpd3d9Buffer;
			int data_len = source_width * stretch_pow_x;
			
			for(int y = 0; y < source_height * stretch_pow_y; y++) {
				for(int i = 0; i < data_len; i++) {
					out[i] = src[i];
				}
				src -= data_len;
				out += data_len;
			}
		}
		// unlock offscreen surface
		lpd3d9Buffer = NULL;
		lpd3d9OffscreenSurface->UnlockRect();
	}
	
	// invalidate window
	InvalidateRect(main_window_handle, NULL, first_invalidate);
	UpdateWindow(main_window_handle);
	self_invalidate = true;
	
	// record avi file
	if(now_rec_video) {
		static double frames = 0;
		static int prev_video_fps = -1;
#ifdef SUPPORT_VARIABLE_TIMING
		static double prev_vm_fps = -1;
		double vm_fps = vm->frame_rate();
		if(prev_video_fps != rec_video_fps || prev_vm_fps != vm_fps) {
			prev_video_fps = rec_video_fps;
			prev_vm_fps = vm_fps;
			frames = vm_fps / rec_video_fps;
		}
#else
		if(prev_video_fps != rec_video_fps) {
			prev_video_fps = rec_video_fps;
			frames = FRAMES_PER_SEC / rec_video_fps;
		}
#endif
		int counter = 0;
		if(use_video_thread) {
			while(rec_video_run_frames > 0) {
				rec_video_run_frames -= frames;
				rec_video_frames += frames;
				counter++;
			}
			if(counter != 0) {
				if(hVideoThread != (HANDLE)0) {
					if(video_thread_param.result == 0) {
						WaitForSingleObject(hVideoThread, INFINITE);
					}
					hVideoThread = (HANDLE)0;
					
					if(video_thread_param.result == RESULT_FULL) {
						stop_rec_video();
						if(!start_rec_video(-1)) {
							return 0;
						}
					} else if(video_thread_param.result == RESULT_ERROR) {
						stop_rec_video();
						return 0;
					}
				}
				BitBlt(hdcDibRec, 0, 0, source_width, source_height, hdcDibSource, 0, 0, SRCCOPY);
				video_thread_param.frames += counter;
				video_thread_param.result = 0;
				if((hVideoThread = (HANDLE)_beginthreadex(NULL, 0, rec_video_thread, &video_thread_param, 0, NULL)) == (HANDLE)0) {
					stop_rec_video();
					return 0;
				}
			}
		} else {
			while(rec_video_run_frames > 0) {
				LONG lBytesWritten;
				if(AVIStreamWrite(pAVICompressed, lAVIFrames++, 1, (LPBYTE)lpBmpSource, pbmInfoHeader->biSizeImage, AVIIF_KEYFRAME, NULL, &lBytesWritten) == AVIERR_OK) {
					// if avi file size > (2GB - 16MB), create new avi file
					if((dwAVIFileSize += lBytesWritten) >= 2130706432) {
						stop_rec_video();
						if(!start_rec_video(-1)) {
							break;
						}
					}
					rec_video_run_frames -= frames;
					rec_video_frames += frames;
					counter++;
				} else {
					stop_rec_video();
					break;
				}
			}
		}
		return counter;
	} else {
		return 1;
	}
#endif
   return 0;
}



scrntype* EMU::screen_buffer(int y)
{
//	if(use_d3d9 && lpd3d9Buffer != NULL && render_to_d3d9Buffer && !now_rec_video) {
//		return lpd3d9Buffer + screen_width * y;
//	}
//	return lpBmp + screen_width * (screen_height - y - 1);
   Uint32 *p = pPseudoVram;
   if((y >= SCREEN_HEIGHT) || (y < 0)) return NULL;
   bDrawLine[y] = true;;

   p = &(p[y * SCREEN_WIDTH]);
   return p;
}
#if defined(_USE_AGAR) || defined(_USE_SDL)
void EMU::update_screen(AG_Widget *target)
#else
void EMU::update_screen(HDC hdc)
#endif
{
   // UpdateScreen
   if(hScreenWidget != NULL) {
	AG_Redraw(hScreenWidget);
   }
	
   

#if 0


# ifdef USE_BITMAP
	if(first_invalidate || !self_invalidate) {
		HDC hmdc = CreateCompatibleDC(hdc);
		HBITMAP hBitmap = LoadBitmap(instance_handle, _T("IDI_BITMAP1"));
		BITMAP bmp;
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		int w = (int)bmp.bmWidth;
		int h = (int)bmp.bmHeight;
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hmdc, hBitmap);
		BitBlt(hdc, 0, 0, w, h, hmdc, 0, 0, SRCCOPY);
		SelectObject(hmdc, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hmdc);
	}
#endif
	if(first_draw_screen) {
#ifdef USE_LED
		// 7-seg LEDs
		for(int i = 0; i < MAX_LEDS; i++) {
			int x = leds[i].x;
			int y = leds[i].y;
			int w = leds[i].width;
			int h = leds[i].height;
			BitBlt(hdc, x, y, w, h, hdcDib, x, y, SRCCOPY);
		}
#else
#ifdef USE_ACCESS_LAMP
		// get access lamps status of drives
		int status = vm->access_lamp() & 7;
		static int prev_status = 0;
		bool render_in = (status != 0);
		bool render_out = (prev_status != status);
		prev_status = status;
		
		COLORREF crColor = RGB((status & 1) ? 255 : 0, (status & 2) ? 255 : 0, (status & 4) ? 255 : 0);
		int right_bottom_x = screen_dest_x + stretched_width;
		int right_bottom_y = screen_dest_y + stretched_height;
#endif
		// standard screen
		if(use_d3d9) {
			LPDIRECT3DSURFACE9 lpd3d9BackSurface = NULL;
			if(lpd3d9Device != NULL && lpd3d9Device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &lpd3d9BackSurface) == D3D_OK && lpd3d9BackSurface != NULL) {
				RECT rectSrc = { 0, 0, source_width * stretch_pow_x, source_height * stretch_pow_y };
				RECT rectDst = { screen_dest_x, screen_dest_y, screen_dest_x + stretched_width, screen_dest_y + stretched_height };
				
				lpd3d9Device->UpdateSurface(lpd3d9OffscreenSurface, NULL, lpd3d9Surface, NULL);
				lpd3d9Device->StretchRect(lpd3d9Surface, &rectSrc, lpd3d9BackSurface, &rectDst, stretch_screen ? D3DTEXF_LINEAR : D3DTEXF_POINT);
#ifdef USE_ACCESS_LAMP
				// draw access lamps
				if(render_in || render_out) {
					HDC hDC = 0;
					for(int y = display_height - 6; y < display_height; y++) {
						for(int x = display_width - 6; x < display_width; x++) {
							if((x < right_bottom_x && y < right_bottom_y) ? render_in : render_out) {
								if(hDC == 0 && lpd3d9BackSurface->GetDC(&hDC) != D3D_OK) {
									goto quit;
								}
								SetPixelV(hDC, x, y, crColor);
							}
						}
					}
quit:
					if(hDC != 0) {
						lpd3d9BackSurface->ReleaseDC(hDC);
					}
				}
#endif
				lpd3d9BackSurface->Release();
				lpd3d9Device->Present(NULL, NULL, NULL, NULL);
			}
		} else {
			if(stretch_screen) {
				BitBlt(hdc, screen_dest_x, screen_dest_y, stretched_width, stretched_height, hdcDibStretch2, 0, 0, SRCCOPY);
			} else if(stretched_width == source_width && stretched_height == source_height) {
				BitBlt(hdc, screen_dest_x, screen_dest_y, stretched_width, stretched_height, hdcDibSource, 0, 0, SRCCOPY);
			} else {
				StretchBlt(hdc, screen_dest_x, screen_dest_y, stretched_width, stretched_height, hdcDibSource, 0, 0, source_width, source_height, SRCCOPY);
			}
#ifdef USE_ACCESS_LAMP
			// draw access lamps
			if(render_in || render_out) {
				for(int y = display_height - 6; y < display_height; y++) {
					for(int x = display_width - 6; x < display_width; x++) {
						if((x < right_bottom_x && y < right_bottom_y) ? render_in : render_out) {
							SetPixelV(hdc, x, y, crColor);
						}
					}
				}
			}
#endif
		}
#endif
		first_invalidate = self_invalidate = false;
	}
#endif
}

void EMU::capture_screen()
{
#if 0
     if(use_d3d9 && render_to_d3d9Buffer && !now_rec_video) {
		// virtual machine may render screen to d3d9 buffer directly...
		vm->draw_screen();
	}
	
	// create file name
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);
	
	_TCHAR file_name[_MAX_PATH];
	_stprintf(file_name, _T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.bmp"), sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
	
	// create bitmap
	BITMAPFILEHEADER bmFileHeader = { (WORD)(TEXT('B') | TEXT('M') << 8) };
	bmFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmFileHeader.bfSize = bmFileHeader.bfOffBits + pbmInfoHeader->biSizeImage;
	
	DWORD dwSize;
	HANDLE hFile = CreateFile(bios_path(file_name), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	WriteFile(hFile, &bmFileHeader, sizeof(BITMAPFILEHEADER), &dwSize, NULL);
	WriteFile(hFile, lpDibSource, sizeof(BITMAPINFOHEADER), &dwSize, NULL);
	WriteFile(hFile, lpBmpSource, pbmInfoHeader->biSizeImage, &dwSize, NULL);
	CloseHandle(hFile);
#endif
}

bool EMU::start_rec_video(int fps)
{
#if 0
       if(fps > 0) {
		rec_video_fps = fps;
		rec_video_run_frames = rec_video_frames = 0;
	} else {
		fps = rec_video_fps;
	}
	bool show_dialog = (fps > 0);
	
	// create file name
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);
	
	_stprintf(video_file_name, _T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.avi"), sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
	
	// initialize vfw
	AVIFileInit();
	if(AVIFileOpen(&pAVIFile, bios_path(video_file_name), OF_WRITE | OF_CREATE, NULL) != AVIERR_OK) {
		return false;
	}
	use_video_thread = false;
	
	// stream header
	AVISTREAMINFO strhdr;
	memset(&strhdr, 0, sizeof(strhdr));
	strhdr.fccType = streamtypeVIDEO;	// vids
	strhdr.fccHandler = 0;
	strhdr.dwScale = 1;
	strhdr.dwRate = fps;
	strhdr.dwSuggestedBufferSize = pbmInfoHeader->biSizeImage;
	SetRect(&strhdr.rcFrame, 0, 0, source_width, source_height);
	if(AVIFileCreateStream(pAVIFile, &pAVIStream, &strhdr) != AVIERR_OK) {
		stop_rec_video();
		return false;
	}
	
	// compression
	AVICOMPRESSOPTIONS FAR * pOpts[1];
	pOpts[0] = &opts;
	if(show_dialog && !AVISaveOptions(main_window_handle, ICMF_CHOOSE_KEYFRAME | ICMF_CHOOSE_DATARATE, 1, &pAVIStream, (LPAVICOMPRESSOPTIONS FAR *)&pOpts)) {
		AVISaveOptionsFree(1, (LPAVICOMPRESSOPTIONS FAR *)&pOpts);
		stop_rec_video();
		return false;
	}
	if(AVIMakeCompressedStream(&pAVICompressed, pAVIStream, &opts, NULL) != AVIERR_OK) {
		stop_rec_video();
		return false;
	}
	if(AVIStreamSetFormat(pAVICompressed, 0, &lpDibSource->bmiHeader, lpDibSource->bmiHeader.biSize + lpDibSource->bmiHeader.biClrUsed * sizeof(RGBQUAD)) != AVIERR_OK) {
		stop_rec_video();
		return false;
	}
	dwAVIFileSize = 0;
	lAVIFrames = 0;
	
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	
	if(info.dwNumberOfProcessors > 1) {
		use_video_thread = true;
		hVideoThread = (HANDLE)0;
		video_thread_param.pAVICompressed = pAVICompressed;
		video_thread_param.lpBmpSource = lpBmpSource;
		video_thread_param.pbmInfoHeader = pbmInfoHeader;
		video_thread_param.dwAVIFileSize = 0;
		video_thread_param.lAVIFrames = 0;
		video_thread_param.frames = 0;
		video_thread_param.result = 0;
		
		HDC hdc = GetDC(main_window_handle);
		create_dib_section(hdc, source_width, source_height, &hdcDibRec, &hBmpRec, &hOldBmpRec, &lpBufRec, &lpBmpRec, &lpDibRec);
		ReleaseDC(main_window_handle, hdc);
	}
	now_rec_video = true;
	return true;
#endif
}

void EMU::stop_rec_video()
{
#if 0
        // release thread
	if(use_video_thread) {
		if(hVideoThread != (HANDLE)0) {
			WaitForSingleObject(hVideoThread, INFINITE);
			hVideoThread = (HANDLE)0;
		}
		if(hdcDibRec) {
			release_dib_section(hdcDibRec, hBmpRec, hOldBmpRec, lpBufRec);
		}
	}
	
	// release vfw
	if(pAVIStream) {
		AVIStreamClose(pAVIStream);
	}
	if(pAVICompressed) {
		AVIStreamClose(pAVICompressed);
	}
	if(pAVIFile) {
		AVIFileClose(pAVIFile);
		AVIFileExit();
	}
	pAVIStream = NULL;
	pAVICompressed = NULL;
	pAVIFile = NULL;
	
	// repair header
	if(now_rec_video) {
		FILE* fp = _tfopen(bios_path(video_file_name), _T("r+b"));
		if(fp != NULL) {
			// copy fccHandler
			uint8 buf[4];
			fseek(fp, 0xbc, SEEK_SET);
			if(ftell(fp) == 0xbc) {
				fread(buf, 4, 1, fp);
				fseek(fp, 0x70, SEEK_SET);
				fwrite(buf, 4, 1, fp);
			}
			fclose(fp);
		}
	}
	now_rec_video = false;
#endif
}

void EMU::restart_rec_video()
{
	bool tmp = now_rec_video;
	stop_rec_video();
	if(tmp) start_rec_video(-1);
}

void *rec_video_thread(void *lpx)
{
#if 0
   volatile video_thread_t *p = (video_thread_t *)lpx;
	LONG lBytesWritten;
	int result = RESULT_SUCCESS;
	
	while(p->frames > 0) {
		if(AVIStreamWrite(p->pAVICompressed, p->lAVIFrames++, 1, (LPBYTE)p->lpBmpSource, p->pbmInfoHeader->biSizeImage, AVIIF_KEYFRAME, NULL, &lBytesWritten) == AVIERR_OK) {
			p->frames--;
			// if avi file size > (2GB - 16MB), create new avi file
			if((p->dwAVIFileSize += lBytesWritten) >= 2130706432) {
				result = RESULT_FULL;
				break;
			}
		} else {
			result = RESULT_ERROR;
			break;
		}
	}
	p->result = result;
	_endthreadex(0);
	return 0;
#endif
}

