/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 screen ]
*/

#include "emu.h"
#include "vm/vm.h"
#include "qt_main.h"
#include "qt_gldraw.h"
#include "agar_logger.h"
//#include "menuclasses.h"
#include "mainwidget.h"
#include "emu_thread.h"

#define RESULT_SUCCESS	1
#define RESULT_FULL	2
#define RESULT_ERROR	3


void *rec_video_thread(void *lpx);

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
	use_GL = true;
	use_SDLFB = false;
	wait_vsync = false;
        // *nix only, need to change on WIndows.
        pPseudoVram = new QImage(SCREEN_WIDTH, SCREEN_HEIGHT, QImage::Format_ARGB32);
	QColor fillcolor;
	fillcolor.setRgb(0, 0, 0, 0);
	pPseudoVram->fill(fillcolor);
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
	if(pPseudoVram != NULL) delete pPseudoVram;
	pPseudoVram = NULL;
	// stop video recording
	//stop_rec_video();
}



int EMU::get_window_width(int mode)
{
#ifdef USE_SCREEN_ROTATE
	if(config.rotate_type) {
		return window_height + screen_height_aspect * mode;
	}
#endif
	return window_width + screen_width_aspect * mode;
}

int EMU::get_window_height(int mode)
{
#ifdef USE_SCREEN_ROTATE
	if(config.rotate_type) {
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
   
	if(width != -1 && (display_width != width || display_height != height)) {
		display_width = width;
		display_height = height;
		display_size_changed = stretch_changed = true;
	}
#ifdef USE_SCREEN_ROTATE
	if(config.rotate_type) {
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
	if(stretch_pow_x != new_pow_x || stretch_pow_y != new_pow_y) {
		stretch_pow_x = new_pow_x;
		stretch_pow_y = new_pow_y;
		stretch_changed = true;
	}
	if(!stretch_changed && !display_size_changed) return;
	AGAR_DebugLog(AGAR_LOG_DEBUG, "Set display size");
	AGAR_DebugLog(AGAR_LOG_DEBUG, "       to %d x %d", width, height);

	if(main_window_handle != NULL) {
		get_parent_handler()->resize_screen(screen_width, screen_height, stretched_width, stretched_height);
	}
	first_draw_screen = false;
	first_invalidate = true;
	screen_size_changed = false;
}

EmuThreadClass *EMU::get_parent_handler(void)
{
	return parent_thread_handler;
}

void EMU::set_parent_handler(EmuThreadClass *p)
{
	parent_thread_handler = p;
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
	get_parent_handler()->resize_screen(screen_width, screen_height, stretched_width, stretched_height);
}


int EMU::draw_screen()
{
	// -> qt_gldraw.cpp , qt_glutil.cpp and qt_glevents.cpp within src/qt/common/ .
	// draw screen
	if(pPseudoVram != NULL) vm->draw_screen();
	// screen size was changed in vm->draw_screen()
	if(screen_size_changed) {
		screen_size_changed = false;
		//		return 0;
	}
	return 1;
}



scrntype* EMU::screen_buffer(int y)
{
	uchar *p = NULL;
	if((pPseudoVram != NULL) && (y < SCREEN_HEIGHT) && (y >= 0)) p = pPseudoVram->scanLine(y);
	return (scrntype *)p;
}

void EMU::update_screen()
{
	// UpdateScreen
	// -> qt_gldraw.cpp , qt_glutil.cpp and qt_glevents.cpp within src/qt/common/ .
}

void EMU::capture_screen()
{
	// -> qt_gldraw.cpp , qt_glutil.cpp and qt_glevents.cpp within src/qt/common/ .
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
#endif // #if 0
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

