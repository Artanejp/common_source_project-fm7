/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2015.11.20-

	[ win32 dependent ]
*/

#ifndef _WIN32_OSD_H_
#define _WIN32_OSD_H_

#define DIRECTSOUND_VERSION	0x900
#define DIRECT3D_VERSION	0x900
#define DIRECTINPUT_VERSION	0x500

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <process.h>
#include <gdiplus.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <d3d9types.h>
#include <vfw.h>
#include <dsound.h>
#include <dinput.h>
#include "../vm/vm.h"
//#include "../emu.h"
#include "../config.h"

#ifdef USE_SOCKET
#include <winsock.h>
#pragma comment(lib, "wsock32.lib")
#endif
#pragma comment(lib, "Gdiplus.lib")
using namespace Gdiplus;
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "vfw32.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dinput.lib")
#pragma comment(lib, "dxguid.lib")

#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
#pragma comment(lib, "strmiids.lib")
#include <dshow.h>
//#include <qedit.h>
EXTERN_C const CLSID CLSID_SampleGrabber;
EXTERN_C const CLSID CLSID_NullRenderer;
EXTERN_C const IID IID_ISampleGrabberCB;
MIDL_INTERFACE("0579154A-2B53-4994-B0D0-E773148EFF85")
ISampleGrabberCB : public IUnknown {
public:
	virtual HRESULT STDMETHODCALLTYPE SampleCB( double SampleTime,IMediaSample *pSample) = 0;
	virtual HRESULT STDMETHODCALLTYPE BufferCB( double SampleTime,BYTE *pBuffer,long BufferLen) = 0;
};
EXTERN_C const IID IID_ISampleGrabber;
MIDL_INTERFACE("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")
ISampleGrabber : public IUnknown {
public:
	virtual HRESULT STDMETHODCALLTYPE SetOneShot( BOOL OneShot) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetMediaType( const AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType( AM_MEDIA_TYPE *pType) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetBufferSamples( BOOL BufferThem) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer( /* [out][in] */ long *pBufferSize,/* [out] */ long *pBuffer) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetCurrentSample( /* [retval][out] */ IMediaSample **ppSample) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetCallback( ISampleGrabberCB *pCallback,long WhichMethodToCallback) = 0;
};
#endif
#ifdef USE_MOVIE_PLAYER
class CMySampleGrabberCB : public ISampleGrabberCB {
private:
	VM *vm;
public:
	CMySampleGrabberCB(VM *vm_ptr)
	{
		vm = vm_ptr;
	}
	STDMETHODIMP_(ULONG) AddRef()
	{
		return 2;
	}
	STDMETHODIMP_(ULONG) Release()
	{
		return 1;
	}
	STDMETHODIMP QueryInterface(REFIID riid, void **ppv)
	{
		if(riid == IID_ISampleGrabberCB || riid == IID_IUnknown) {
			*ppv = (void *) static_cast<ISampleGrabberCB*>(this);
			return NOERROR;
		}
		return E_NOINTERFACE;
	}
	STDMETHODIMP SampleCB(double SampleTime, IMediaSample *pSample)
	{
		return S_OK;
	}
	STDMETHODIMP BufferCB(double dblSampleTime, BYTE *pBuffer, long lBufferSize)
	{
		vm->movie_sound_callback(pBuffer, lBufferSize);
		return S_OK;
	}
};
#endif

#define WM_RESIZE  (WM_USER + 1)
#define WM_SOCKET0 (WM_USER + 2)
#define WM_SOCKET1 (WM_USER + 3)
#define WM_SOCKET2 (WM_USER + 4)
#define WM_SOCKET3 (WM_USER + 5)

#ifdef USE_SOCKET
#define SOCKET_MAX 4
#define SOCKET_BUFFER_MAX 0x100000
#endif

#ifdef USE_VIDEO_CAPTURE
#define MAX_CAPTURE_DEVS 8
#endif

#define SUPPORT_WIN32_DLL

// check memory leaks
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

typedef struct bitmap_s {
	// common
	inline bool initialized()
	{
		return (hdcDib != NULL);
	}
	inline scrntype* get_buffer(int y)
	{
		return lpBmp + width * (height - y - 1);
	}
	int width, height;
	// win32 dependent
	HDC hdcDib;
	HBITMAP hBmp, hOldBmp;
	LPBYTE lpBuf;
	scrntype* lpBmp;
	LPBITMAPINFO lpDib;
} bitmap_t;

typedef struct font_s {
	// common
	inline bool initialized()
	{
		return (hFont != NULL);
	}
	_TCHAR family[64];
	int width, height, rotate;
	bool bold, italic;
	// win32 dependent
	HFONT hFont;
} font_t;

typedef struct pen_s {
	// common
	inline bool initialized()
	{
		return (hPen != NULL);
	}
	int width;
	uint8 r, g, b;
	// win32 dependent
	HPEN hPen;
} pen_t;

typedef struct {
	PAVISTREAM pAVICompressed;
	scrntype* lpBmp;
	LPBITMAPINFOHEADER pbmInfoHeader;
	DWORD dwAVIFileSize;
	LONG lAVIFrames;
	int frames;
	int result;
} rec_video_thread_param_t;

class FIFO;
class FILEIO;

class OSD
{
private:
	int lock_count;
	
	// console
	HANDLE hStdIn, hStdOut;
	
	// input
	void initialize_input();
	void release_input();
	void key_down_sub(int code, bool repeat);
	void key_up_sub(int code);
	
	LPDIRECTINPUT lpdi;
	LPDIRECTINPUTDEVICE lpdikey;
//	LPDIRECTINPUTDEVICE lpdijoy;
	bool dinput_key_ok;
//	bool dinput_joy_ok;
	
	uint8 keycode_conv[256];
	uint8 key_status[256];	// windows key code mapping
	uint8 key_dik_prev[256];
#ifdef USE_SHIFT_NUMPAD_KEY
	uint8 key_converted[256];
	bool key_shift_pressed, key_shift_released;
#endif
	bool lost_focus;
	
	uint32 joy_status[2];	// joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4- = buttons
	int joy_num;
	uint32 joy_mask[2];
	
	int mouse_status[3];	// x, y, button (b0 = left, b1 = right)
	bool mouse_enabled;
	
#ifdef USE_AUTO_KEY
	FIFO* autokey_buffer;
	int autokey_phase, autokey_shift;
	int autokey_table[256];
#endif
	
	// screen
	void initialize_screen();
	void release_screen();
	void initialize_screen_buffer(bitmap_t *buffer, int width, int height, int mode);
	void release_screen_buffer(bitmap_t *buffer);
#ifdef USE_CRT_FILTER
	void apply_crt_fileter_to_screen_buffer(bitmap_t *source, bitmap_t *dest);
	void apply_crt_filter_x3_y3(bitmap_t *source, bitmap_t *dest);
	void apply_crt_filter_x3_y2(bitmap_t *source, bitmap_t *dest);
	void apply_crt_filter_x2_y3(bitmap_t *source, bitmap_t *dest);
	void apply_crt_filter_x2_y2(bitmap_t *source, bitmap_t *dest);
	void apply_crt_filter_x1_y1(bitmap_t *source, bitmap_t *dest);
#endif
#ifdef USE_SCREEN_ROTATE
	void rotate_screen_buffer(bitmap_t *source, bitmap_t *dest);
#endif
	void stretch_screen_buffer(bitmap_t *source, bitmap_t *dest);
	bool initialize_d3d9();
	bool initialize_d3d9_surface(bitmap_t *buffer);
	void release_d3d9();
	void release_d3d9_surface();
	void copy_to_d3d9_surface(bitmap_t *buffer);
	int add_video_frames();
	
	bitmap_t vm_screen_buffer;
#ifdef USE_CRT_FILTER
	bitmap_t filtered_screen_buffer;
	bitmap_t tmp_filtered_screen_buffer;
#endif
#ifdef USE_SCREEN_ROTATE
	bitmap_t rotated_screen_buffer;
#endif
	bitmap_t stretched_screen_buffer;
	bitmap_t shrinked_screen_buffer;
	bitmap_t video_screen_buffer;
	
	bitmap_t* draw_screen_buffer;
	
	int host_window_width, host_window_height;
	bool host_window_mode;
	int base_window_width, base_window_height;
	int vm_screen_width, vm_screen_height, vm_screen_width_aspect, vm_screen_height_aspect;
	int draw_screen_width, draw_screen_height;
	
	Gdiplus::GdiplusStartupInput gdiSI;
	ULONG_PTR gdiToken;
	
	LPDIRECT3D9 lpd3d9;
	LPDIRECT3DDEVICE9 lpd3d9Device;
	LPDIRECT3DSURFACE9 lpd3d9Surface;
	LPDIRECT3DSURFACE9 lpd3d9OffscreenSurface;
	
	_TCHAR video_file_path[_MAX_PATH];
	int rec_video_fps;
	double rec_video_run_frames;
	double rec_video_frames;
	
	LPBITMAPINFO lpDibRec;
	PAVIFILE pAVIFile;
	PAVISTREAM pAVIStream;
	PAVISTREAM pAVICompressed;
	AVICOMPRESSOPTIONS AVIOpts;
	DWORD dwAVIFileSize;
	LONG lAVIFrames;
	HANDLE hVideoThread;
	rec_video_thread_param_t rec_video_thread_param;
	
	bool first_draw_screen;
	bool first_invalidate;
	bool self_invalidate;
	
	// sound
	void initialize_sound(int rate, int samples);
	void release_sound();
	
	int sound_rate, sound_samples;
	bool sound_ok, sound_started, now_mute;
	
	LPDIRECTSOUND lpds;
	LPDIRECTSOUNDBUFFER lpdsPrimaryBuffer, lpdsSecondaryBuffer;
	bool sound_first_half;
	
	_TCHAR sound_file_path[_MAX_PATH];
	FILEIO* rec_sound_fio;
	int rec_sound_bytes;
	int rec_sound_buffer_ptr;
	
	// video device
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	void initialize_video();
	void release_video();
	
	IGraphBuilder *pGraphBuilder;
	IBaseFilter *pVideoBaseFilter;
	IBaseFilter *pCaptureBaseFilter;
	ICaptureGraphBuilder2 *pCaptureGraphBuilder2;
	ISampleGrabber *pVideoSampleGrabber;
	IBaseFilter *pSoundBaseFilter;
	ISampleGrabber *pSoundSampleGrabber;
	CMySampleGrabberCB *pSoundCallBack;
	IMediaControl *pMediaControl;
	IMediaSeeking *pMediaSeeking;
	IMediaPosition *pMediaPosition;
	IVideoWindow *pVideoWindow;
	IBasicVideo *pBasicVideo;
	IBasicAudio *pBasicAudio;
	bool bTimeFormatFrame;
	bool bVerticalReversed;
	
	bitmap_t dshow_screen_buffer;
	int direct_show_width, direct_show_height;
	bool direct_show_mute[2];
#endif
#ifdef USE_MOVIE_PLAYER
	double movie_frame_rate;
	int movie_sound_rate;
#endif
#ifdef USE_VIDEO_CAPTURE
	void enum_capture_devs();
	bool connect_capture_dev(int index, bool pin);
	int cur_capture_dev_index;
	int num_capture_devs;
	_TCHAR capture_dev_name[MAX_CAPTURE_DEVS][256];
#endif
	
	// socket
#ifdef USE_SOCKET
	void initialize_socket();
	void release_socket();
	
	int soc[SOCKET_MAX];
	bool is_tcp[SOCKET_MAX];
	struct sockaddr_in udpaddr[SOCKET_MAX];
	int socket_delay[SOCKET_MAX];
	char recv_buffer[SOCKET_MAX][SOCKET_BUFFER_MAX];
	int recv_r_ptr[SOCKET_MAX], recv_w_ptr[SOCKET_MAX];
#endif
	
public:
	OSD()
	{
		lock_count = 0;
	}
	~OSD() {}
	
	// common
	VM* vm;
	
	void initialize(int rate, int samples);
	void release();
	void power_off();
	void suspend();
	void restore();
	void lock_vm();
	void unlock_vm();
	void force_unlock_vm();
	void sleep(uint32 ms);
	
	// common console
	void open_console(_TCHAR* title);
	void close_console();
	unsigned int get_console_code_page();
	bool is_console_active();
	void set_console_text_attribute(unsigned short attr);
	void write_console(_TCHAR* buffer, unsigned int length);
	int read_console_input(_TCHAR* buffer);
	
	// common input
	void update_input();
	void key_down(int code, bool repeat);
	void key_up(int code);
	void key_lost_focus()
	{
		lost_focus = true;
	}
#ifdef ONE_BOARD_MICRO_COMPUTER
	void press_button(int num);
#endif
	void enable_mouse();
	void disenable_mouse();
	void toggle_mouse();
	bool get_mouse_enabled()
	{
		return mouse_enabled;
	}
#ifdef USE_AUTO_KEY
	void start_auto_key();
	void stop_auto_key();
	bool now_auto_key()
	{
		return (autokey_phase != 0);
	}
#endif
	uint8* key_buffer()
	{
		return key_status;
	}
	uint32* joy_buffer()
	{
		return joy_status;
	}
	int* mouse_buffer()
	{
		return mouse_status;
	}
	
	// common screen
	int get_window_width(int mode);
	int get_window_height(int mode);
	void set_window_size(int window_width, int window_height, bool window_mode);
	void set_vm_screen_size(int width, int height, int width_aspect, int height_aspect, int window_width, int window_height);
	scrntype* get_vm_screen_buffer(int y);
	int draw_screen();
#ifdef ONE_BOARD_MICRO_COMPUTER
	void reload_bitmap()
	{
		first_invalidate = true;
	}
#endif
	void capture_screen();
	bool start_rec_video(int fps);
	void stop_rec_video();
	void restart_rec_video();
	void add_extra_frames(int extra_frames);
	bool now_rec_video;
#ifdef USE_CRT_FILTER
	bool screen_skip_line;
#endif
	
	// common sound
	void update_sound(int* extra_frames);
	void mute_sound();
	void stop_sound();
	void start_rec_sound();
	void stop_rec_sound();
	void restart_rec_sound();
	bool now_rec_sound;
	
	// common video device
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	void get_video_buffer();
	void mute_video_dev(bool l, bool r);
#endif
#ifdef USE_MOVIE_PLAYER
	bool open_movie_file(const _TCHAR* file_path);
	void close_movie_file();
	void play_movie();
	void stop_movie();
	void pause_movie();
	double get_movie_frame_rate()
	{
		return movie_frame_rate;
	}
	int get_movie_sound_rate()
	{
		return movie_sound_rate;
	}
	void set_cur_movie_frame(int frame, bool relative);
	uint32 get_cur_movie_frame();
	bool now_movie_play, now_movie_pause;
#endif
#ifdef USE_VIDEO_CAPTURE
	int get_cur_capture_dev_index()
	{
		return cur_capture_dev_index;
	}
	int get_num_capture_devs()
	{
		return num_capture_devs;
	}
	_TCHAR* get_capture_dev_name(int index)
	{
		return capture_dev_name[index];
	}
	void open_capture_dev(int index, bool pin);
	void close_capture_dev();
	void show_capture_dev_filter();
	void show_capture_dev_pin();
	void show_capture_dev_source();
	void set_capture_dev_channel(int ch);
#endif
	
	// common printer
#ifdef USE_PRINTER
	void create_bitmap(bitmap_t *bitmap, int width, int height);
	void release_bitmap(bitmap_t *bitmap);
	void create_font(font_t *font, const _TCHAR *family, int width, int height, int rotate, bool bold, bool italic);
	void release_font(font_t *font);
	void create_pen(pen_t *pen, int width, uint8 r, uint8 g, uint8 b);
	void release_pen(pen_t *pen);
	void clear_bitmap(bitmap_t *bitmap, uint8 r, uint8 g, uint8 b);
	int get_text_width(bitmap_t *bitmap, font_t *font, const char *text);
	void draw_text_to_bitmap(bitmap_t *bitmap, font_t *font, int x, int y, const char *text, uint8 r, uint8 g, uint8 b);
	void draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen, int sx, int sy, int ex, int ey);
	void draw_rectangle_to_bitmap(bitmap_t *bitmap, int x, int y, int width, int height, uint8 r, uint8 g, uint8 b);
	void draw_point_to_bitmap(bitmap_t *bitmap, int x, int y, uint8 r, uint8 g, uint8 b);
	void stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y, int dest_width, int dest_height, bitmap_t *source, int source_x, int source_y, int source_width, int source_height);
#endif
	void write_bitmap_to_file(bitmap_t *bitmap, const _TCHAR *file_path);
	
	// common socket
#ifdef USE_SOCKET
	int get_socket(int ch)
	{
		return soc[ch];
	}
	void socket_connected(int ch);
	void socket_disconnected(int ch);
	void update_socket();
	bool init_socket_tcp(int ch);
	bool init_socket_udp(int ch);
	bool connect_socket(int ch, uint32 ipaddr, int port);
	void disconnect_socket(int ch);
	bool listen_socket(int ch);
	void send_data_tcp(int ch);
	void send_data_udp(int ch, uint32 ipaddr, int port);
	void send_data(int ch);
	void recv_data(int ch);
#endif
	
	// win32 dependent
	void update_screen(HDC hdc);
	HWND main_window_handle;
	HINSTANCE instance_handle;
	bool vista_or_later;
};

#endif
