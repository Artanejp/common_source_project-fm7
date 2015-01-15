/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 screen ]
*/

#include "emu.h"
#include "vm/vm.h"

#define RESULT_SUCCESS	1
#define RESULT_FULL	2
#define RESULT_ERROR	3

unsigned __stdcall rec_video_thread(void *lpx);

#ifdef USE_CRT_FILTER
static uint8 r0[2048], g0[2048], b0[2048], t0[2048];
static uint8 r1[2048], g1[2048], b1[2048];
#endif

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
	
	// create dib sections
	HDC hdc = GetDC(main_window_handle);
	create_dib_section(hdc, screen_width, screen_height, &hdcDib, &hBmp, &hOldBmp, &lpBuf, &lpBmp, &lpDib);
#ifdef USE_SCREEN_ROTATE
	create_dib_section(hdc, screen_height, screen_width, &hdcDibRotate, &hBmpRotate, &hOldBmpRotate, &lpBufRotate, &lpBmpRotate, &lpDibRotate);
#endif
	ReleaseDC(main_window_handle, hdc);
	
	hdcDibStretch1 = hdcDibStretch2 = NULL;
	hBmpStretch1 = hOldBmpStretch1 = hBmpStretch2 = hOldBmpStretch2 = NULL;
	lpBufStretch1 = lpBufStretch2 = NULL;
	
	// initialize d3d9
	lpd3d9 = NULL;
	lpd3d9Device = NULL;
	lpd3d9Surface = NULL;
	lpd3d9OffscreenSurface = NULL;
	lpd3d9Buffer = NULL;
	render_to_d3d9Buffer = false;
	use_d3d9 = config.use_d3d9;
	wait_vsync = config.wait_vsync;
	
	// initialize video recording
	now_rec_video = false;
	hdcDibRec = NULL;
	pAVIStream = NULL;
	pAVICompressed = NULL;
	pAVIFile = NULL;
	
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

#define release_dib_section(hdcdib, hbmp, holdbmp, lpbuf) { \
	if(hdcdib != NULL && holdbmp != NULL) { \
		SelectObject(hdcdib, holdbmp); \
	} \
	if(hbmp != NULL) { \
		DeleteObject(hbmp); \
		hbmp = NULL; \
	} \
	if(lpbuf != NULL) { \
		GlobalFree(lpbuf); \
		lpbuf = NULL; \
	} \
	if(hdcdib != NULL) { \
		DeleteDC(hdcdib); \
		hdcdib = NULL; \
	} \
}

#define release_d3d9() { \
	if(lpd3d9OffscreenSurface != NULL) { \
		lpd3d9OffscreenSurface->Release(); \
		lpd3d9OffscreenSurface = NULL; \
	} \
	if(lpd3d9Surface != NULL) { \
		lpd3d9Surface->Release(); \
		lpd3d9Surface = NULL; \
	} \
	if(lpd3d9Device != NULL) { \
		lpd3d9Device->Release(); \
		lpd3d9Device = NULL; \
	} \
	if(lpd3d9 != NULL) { \
		lpd3d9->Release(); \
		lpd3d9 = NULL; \
	} \
}

#define release_d3d9_surface() { \
	if(lpd3d9OffscreenSurface != NULL) { \
		lpd3d9OffscreenSurface->Release(); \
		lpd3d9OffscreenSurface = NULL; \
	} \
	if(lpd3d9Surface != NULL) { \
		lpd3d9Surface->Release(); \
		lpd3d9Surface = NULL; \
	} \
}

void EMU::release_screen()
{
	// stop video recording
	stop_rec_video();
	
	// release dib sections
	release_dib_section(hdcDib, hBmp, hOldBmp, lpBuf);
#ifdef USE_SCREEN_ROTATE
	release_dib_section(hdcDibRotate, hBmpRotate, hOldBmpRotate, lpBufRotate);
#endif
	release_dib_section(hdcDibStretch1, hBmpStretch1, hOldBmpStretch1, lpBufStretch1);
	release_dib_section(hdcDibStretch2, hBmpStretch2, hOldBmpStretch2, lpBufStretch2);
	
	// release d3d9
	release_d3d9();
}

void EMU::create_dib_section(HDC hdc, int width, int height, HDC *hdcDib, HBITMAP *hBmp, HBITMAP *hOldBmp, LPBYTE *lpBuf, scrntype **lpBmp, LPBITMAPINFO *lpDib)
{
	*hdcDib = CreateCompatibleDC(hdc);
	*lpBuf = (LPBYTE)GlobalAlloc(GPTR, sizeof(BITMAPINFO));
	*lpDib = (LPBITMAPINFO)(*lpBuf);
	memset(&(*lpDib)->bmiHeader, 0, sizeof(BITMAPINFOHEADER));
	(*lpDib)->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	(*lpDib)->bmiHeader.biWidth = width;
	(*lpDib)->bmiHeader.biHeight = height;
	(*lpDib)->bmiHeader.biPlanes = 1;
#if defined(_RGB555)
	(*lpDib)->bmiHeader.biBitCount = 16;
	(*lpDib)->bmiHeader.biCompression = BI_RGB;
	(*lpDib)->bmiHeader.biSizeImage = width * height * 2;
#elif defined(_RGB565)
	(*lpDib)->bmiHeader.biBitCount = 16;
	(*lpDib)->bmiHeader.biCompression = BI_BITFIELDS;
	LPDWORD lpBf = (LPDWORD)*lpDib->bmiColors;
	lpBf[0] = 0x1f << 11;
	lpBf[1] = 0x3f << 5;
	lpBf[2] = 0x1f << 0;
	(*lpDib)->bmiHeader.biSizeImage = width * height * 2;
#elif defined(_RGB888)
	(*lpDib)->bmiHeader.biBitCount = 32;
	(*lpDib)->bmiHeader.biCompression = BI_RGB;
	(*lpDib)->bmiHeader.biSizeImage = width * height * 4;
#endif
	(*lpDib)->bmiHeader.biXPelsPerMeter = 0;
	(*lpDib)->bmiHeader.biYPelsPerMeter = 0;
	(*lpDib)->bmiHeader.biClrUsed = 0;
	(*lpDib)->bmiHeader.biClrImportant = 0;
	*hBmp = CreateDIBSection(hdc, *lpDib, DIB_RGB_COLORS, (PVOID*)&(*lpBmp), NULL, 0);
	*hOldBmp = (HBITMAP)SelectObject(*hdcDib, *hBmp);
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
RETRY:
	bool display_size_changed = false;
	bool stretch_changed = false;
	int prev_stretched_width = stretched_width;
	int prev_stretched_height = stretched_height;
	
	if(width != -1 && (display_width != width || display_height != height)) {
		display_width = width;
		display_height = height;
		display_size_changed = stretch_changed = true;
	}
	if(use_d3d9 != config.use_d3d9) {
		if(!(use_d3d9 = config.use_d3d9)) {
			release_d3d9();
		}
		display_size_changed = stretch_changed = true;
	}
	if(wait_vsync != config.wait_vsync) {
		wait_vsync = config.wait_vsync;
		display_size_changed = stretch_changed = true;
	}
	
	// virtual machine renders to d3d9 buffer directly???
	render_to_d3d9Buffer = use_d3d9;
	
#ifdef USE_SCREEN_ROTATE
	if(config.monitor_type) {
		hdcDibSource = hdcDibRotate;
		lpBmpSource = lpBmpRotate;
		lpDibSource = lpDibRotate;
		pbmInfoHeader = &lpDibRotate->bmiHeader;
		
		stretch_changed |= (source_width != screen_height);
		stretch_changed |= (source_height != screen_width);
		stretch_changed |= (source_width_aspect != screen_height_aspect);
		stretch_changed |= (source_height_aspect != screen_width_aspect);
		
		source_width = screen_height;
		source_height = screen_width;
		source_width_aspect = screen_height_aspect;
		source_height_aspect = screen_width_aspect;
		
		render_to_d3d9Buffer = false;
	} else {
#endif
		hdcDibSource = hdcDib;
		lpBmpSource = lpBmp;
		lpDibSource = lpDib;
		pbmInfoHeader = &lpDib->bmiHeader;
		
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
	
	if(config.stretch_type == 1 && !window_mode) {
		// fit to full screen (aspect)
		stretched_width = (display_height * source_width_aspect) / source_height_aspect;
		stretched_height = display_height;
		if(stretched_width > display_width) {
			stretched_width = display_width;
			stretched_height = (display_width * source_height_aspect) / source_width_aspect;
		}
	} else if(config.stretch_type == 2 && !window_mode) {
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
	if(stretch_pow_x != 1 || stretch_pow_y != 1) {
		render_to_d3d9Buffer = false;
	}
	
	if(stretch_changed) {
		release_dib_section(hdcDibStretch1, hBmpStretch1, hOldBmpStretch1, lpBufStretch1);
		release_dib_section(hdcDibStretch2, hBmpStretch2, hOldBmpStretch2, lpBufStretch2);
		stretch_screen = false;
		
		if(stretch_pow_x != 1 || stretch_pow_y != 1) {
			HDC hdc = GetDC(main_window_handle);
			create_dib_section(hdc, source_width * stretch_pow_x, source_height * stretch_pow_y, &hdcDibStretch1, &hBmpStretch1, &hOldBmpStretch1, &lpBufStretch1, &lpBmpStretch1, &lpDibStretch1);
			SetStretchBltMode(hdcDibStretch1, COLORONCOLOR);
			if(!use_d3d9) {
				create_dib_section(hdc, stretched_width, stretched_height, &hdcDibStretch2, &hBmpStretch2, &hOldBmpStretch2, &lpBufStretch2, &lpBmpStretch2, &lpDibStretch2);
				SetStretchBltMode(hdcDibStretch2, HALFTONE);
			}
			ReleaseDC(main_window_handle, hdc);
			stretch_screen = true;
		}
		
		if(use_d3d9 && display_size_changed) {
			// release and initialize d3d9
			release_d3d9();
			
			if((lpd3d9 = Direct3DCreate9(D3D_SDK_VERSION)) == NULL) {
				MessageBox(main_window_handle, _T("Failed to initialize Direct3D9"), _T(DEVICE_NAME), MB_OK | MB_ICONWARNING);
				config.use_d3d9 = false;
				goto RETRY;
			} else {
				// initialize present params
				D3DPRESENT_PARAMETERS d3dpp;
				ZeroMemory(&d3dpp, sizeof(d3dpp));
				d3dpp.BackBufferWidth = display_width;
				d3dpp.BackBufferHeight = display_height;
				d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;//D3DFMT_UNKNOWN;
				d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
				d3dpp.hDeviceWindow = main_window_handle;
				d3dpp.Windowed = TRUE;
				d3dpp.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
				d3dpp.PresentationInterval = config.wait_vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;
				
				// create d3d9 device
				HRESULT hr = lpd3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, main_window_handle, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &lpd3d9Device);
				if(hr != D3D_OK) {
					hr = lpd3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, main_window_handle, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &lpd3d9Device);
					if(hr != D3D_OK) {
						hr = lpd3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, main_window_handle, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &lpd3d9Device);
					}
				}
				if(hr != D3D_OK) {
					MessageBox(main_window_handle, _T("Failed to create a Direct3D9 device"), _T(DEVICE_NAME), MB_OK | MB_ICONWARNING);
					config.use_d3d9 = false;
					goto RETRY;
				}
			}
		}
		if(use_d3d9 && lpd3d9Device != NULL) {
			// release and create d3d9 surfaces
			release_d3d9_surface();
			
			HRESULT hr = lpd3d9Device->CreateOffscreenPlainSurface(source_width * stretch_pow_x, source_height * stretch_pow_y, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &lpd3d9Surface, NULL);
			if(hr == D3D_OK) {
				hr = lpd3d9Device->CreateOffscreenPlainSurface(source_width * stretch_pow_x, source_height * stretch_pow_y, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &lpd3d9OffscreenSurface, NULL);
			}
			if(hr == D3D_OK) {
				lpd3d9Device->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 0.0, 0);
			} else {
				MessageBox(main_window_handle, _T("Failed to create a Direct3D9 offscreen surface"), _T(DEVICE_NAME), MB_OK | MB_ICONWARNING);
				config.use_d3d9 = false;
				goto RETRY;
			}
		}
		if(stretch_screen) {
			render_to_d3d9Buffer = false;
		}
	}
	
	first_draw_screen = false;
	first_invalidate = true;
	screen_size_changed = false;
	
#ifdef USE_CRT_FILTER
	memset(r1, 0, sizeof(r1));
	memset(g1, 0, sizeof(g1));
	memset(b1, 0, sizeof(b1));
#endif
}

void EMU::change_screen_size(int sw, int sh, int swa, int sha, int ww, int wh)
{
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
		HDC hdc = GetDC(main_window_handle);
		release_dib_section(hdcDib, hBmp, hOldBmp, lpBuf);
		create_dib_section(hdc, screen_width, screen_height, &hdcDib, &hBmp, &hOldBmp, &lpBuf, &lpBmp, &lpDib);
#ifdef USE_SCREEN_ROTATE
		release_dib_section(hdcDibRotate, hBmpRotate, hOldBmpRotate, lpBufRotate);
		create_dib_section(hdc, screen_height, screen_width, &hdcDibRotate, &hBmpRotate, &hOldBmpRotate, &lpBufRotate, &lpBmpRotate, &lpDibRotate);
#endif
		ReleaseDC(main_window_handle, hdc);
		
		// stop recording
		if(now_rec_video) {
			stop_rec_video();
			stop_rec_sound();
		}
		
		// change the window size
		PostMessage(main_window_handle, WM_RESIZE, 0L, 0L);
	}
}

int EMU::draw_screen()
{
	// don't draw screen before new screen size is applied to buffers
	if(screen_size_changed) {
		return 0;
	}
	
	// check avi file recording timing
	if(now_rec_video && rec_video_run_frames <= 0) {
		return 0;
	}
	
	// lock offscreen surface
	D3DLOCKED_RECT pLockedRect;
	if(use_d3d9 && lpd3d9OffscreenSurface != NULL && lpd3d9OffscreenSurface->LockRect(&pLockedRect, NULL, 0) == D3D_OK) {
		lpd3d9Buffer = (scrntype *)pLockedRect.pBits;
	} else {
		lpd3d9Buffer = NULL;
	}
	
	// draw screen
	vm->draw_screen();
	
	// screen size was changed in vm->draw_screen()
	if(screen_size_changed) {
		// unlock offscreen surface
		if(use_d3d9 && lpd3d9Buffer != NULL) {
			lpd3d9Buffer = NULL;
			lpd3d9OffscreenSurface->UnlockRect();
		}
		return 0;
	}
	
#ifdef USE_SCREEN_ROTATE
	// rotate screen
	if(config.monitor_type) {
		for(int y = 0; y < screen_height; y++) {
			scrntype* src = lpBmp + screen_width * (screen_height - y - 1);
			scrntype* out = lpBmpRotate + screen_height * (screen_width - 1) + (screen_height - y - 1);
			for(int x = 0; x < screen_width; x++) {
				*out = src[x];
				out -= screen_height;
			}
		}
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
}

scrntype* EMU::screen_buffer(int y)
{
	if(use_d3d9 && lpd3d9Buffer != NULL && render_to_d3d9Buffer && !now_rec_video) {
		return lpd3d9Buffer + screen_width * y;
	}
	return lpBmp + screen_width * (screen_height - y - 1);
}

void EMU::update_screen(HDC hdc)
{
#ifdef USE_BITMAP
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
}

void EMU::capture_screen()
{
	if(use_d3d9 && render_to_d3d9Buffer && !now_rec_video) {
		// virtual machine may render screen to d3d9 buffer directly...
		vm->draw_screen();
	}
	
	// create file name
	SYSTEMTIME sTime;
	GetLocalTime(&sTime);
	
	_TCHAR file_name[_MAX_PATH];
	_stprintf_s(file_name, _MAX_PATH, _T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.bmp"), sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
	
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
}

bool EMU::start_rec_video(int fps)
{
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
	
	_stprintf_s(video_file_name, _MAX_PATH, _T("%d-%0.2d-%0.2d_%0.2d-%0.2d-%0.2d.avi"), sTime.wYear, sTime.wMonth, sTime.wDay, sTime.wHour, sTime.wMinute, sTime.wSecond);
	
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
}

void EMU::stop_rec_video()
{
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
		FILE* fp = NULL;
		if(_tfopen_s(&fp, bios_path(video_file_name), _T("r+b")) == 0) {
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
}

void EMU::restart_rec_video()
{
	bool tmp = now_rec_video;
	stop_rec_video();
	if(tmp) start_rec_video(-1);
}

unsigned __stdcall rec_video_thread(void *lpx)
{
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
}

