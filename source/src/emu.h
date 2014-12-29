/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 emulation i/f ]
*/

#ifndef _EMU_H_
#define _EMU_H_

// DirectX
#define DIRECTSOUND_VERSION 0x900
#define DIRECT3D_VERSION 0x900

// for debug
//#define _DEBUG_LOG
#ifdef _DEBUG_LOG
	// output fdc debug log
//	#define _FDC_DEBUG_LOG
	// output i/o debug log
//	#define _IO_DEBUG_LOG
#endif

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <process.h>
#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "config.h"
#include "vm/vm.h"

#define WM_RESIZE  (WM_USER + 1)
#define WM_SOCKET0 (WM_USER + 2)
#define WM_SOCKET1 (WM_USER + 3)
#define WM_SOCKET2 (WM_USER + 4)
#define WM_SOCKET3 (WM_USER + 5)

#if defined(USE_LASER_DISC) || defined(USE_VIDEO_CAPTURE)
#define USE_DIRECT_SHOW
#endif
#ifdef USE_VIDEO_CAPTURE
#define MAX_CAPTURE_DEVS 8
#endif

#ifndef SCREEN_WIDTH_ASPECT
#define SCREEN_WIDTH_ASPECT SCREEN_WIDTH
#endif
#ifndef SCREEN_HEIGHT_ASPECT
#define SCREEN_HEIGHT_ASPECT SCREEN_HEIGHT
#endif
#ifndef WINDOW_WIDTH
#define WINDOW_WIDTH SCREEN_WIDTH_ASPECT
#endif
#ifndef WINDOW_HEIGHT
#define WINDOW_HEIGHT SCREEN_HEIGHT_ASPECT
#endif

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#include <d3d9.h>
#include <d3dx9.h>
#include <d3d9types.h>

#include <dsound.h>
#include <vfw.h>

#ifdef USE_DIRECT_SHOW
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
#ifdef USE_LASER_DISC
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
#endif

#ifdef USE_SOCKET
#include <winsock.h>
#endif

// check memory leaks
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#ifdef USE_FD1
#define MAX_D88_BANKS 64
#endif

#ifdef USE_SOCKET
#define SOCKET_MAX 4
#define SOCKET_BUFFER_MAX 0x100000
#endif

class EMU;
class FIFO;
class FILEIO;

typedef struct video_thread_t {
	PAVISTREAM pAVICompressed;
	scrntype* lpBmpSource;
	LPBITMAPINFOHEADER pbmInfoHeader;
	DWORD dwAVIFileSize;
	LONG lAVIFrames;
	int frames;
	int result;
} video_thread_t;

#ifdef USE_DEBUGGER
typedef struct debugger_thread_t {
	EMU *emu;
	VM *vm;
	int cpu_index;
	bool running;
	bool request_terminate;
} debugger_thread_t;
#endif

class EMU
{
protected:
	VM* vm;
private:
	// ----------------------------------------
	// input
	// ----------------------------------------
	void initialize_input();
	void release_input();
	void update_input();
	
	uint8 keycode_conv[256];
	uint8 key_status[256];	// windows key code mapping
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
#endif
	
	// ----------------------------------------
	// screen
	// ----------------------------------------
	void initialize_screen();
	void release_screen();
	void create_dib_section(HDC hdc, int width, int height, HDC *hdcDib, HBITMAP *hBmp, HBITMAP *hOldBmp, LPBYTE *lpBuf, scrntype **lpBmp, LPBITMAPINFO *lpDib);
	
	// screen settings
	int screen_width, screen_height;
	int screen_width_aspect, screen_height_aspect;
	int window_width, window_height;
	int display_width, display_height;
	bool screen_size_changed;
	
	HDC hdcDibSource;
	scrntype* lpBmpSource;
	LPBITMAPINFO lpDibSource;
	LPBITMAPINFOHEADER pbmInfoHeader;
	
	int source_width, source_height;
	int source_width_aspect, source_height_aspect;
	int stretched_width, stretched_height;
	int stretch_pow_x, stretch_pow_y;
	int screen_dest_x, screen_dest_y;
	bool stretch_screen;
	
	// update flags
	bool first_draw_screen;
	bool first_invalidate;
	bool self_invalidate;
	
	// screen buffer
	HDC hdcDib;
	HBITMAP hBmp, hOldBmp;
	LPBYTE lpBuf;
	scrntype* lpBmp;
	LPBITMAPINFO lpDib;
	
#ifdef USE_SCREEN_ROTATE
	// rotate buffer
	HDC hdcDibRotate;
	HBITMAP hBmpRotate, hOldBmpRotate;
	LPBYTE lpBufRotate;
	scrntype* lpBmpRotate;
	LPBITMAPINFO lpDibRotate;
#endif
	
	// stretch buffer
	HDC hdcDibStretch1;
	HBITMAP hBmpStretch1, hOldBmpStretch1;
	LPBYTE lpBufStretch1;
	scrntype* lpBmpStretch1;
	LPBITMAPINFO lpDibStretch1;
	
	HDC hdcDibStretch2;
	HBITMAP hBmpStretch2, hOldBmpStretch2;
	LPBYTE lpBufStretch2;
	scrntype* lpBmpStretch2;
	LPBITMAPINFO lpDibStretch2;
	
	// for direct3d9
	LPDIRECT3D9 lpd3d9;
	LPDIRECT3DDEVICE9 lpd3d9Device;
	LPDIRECT3DSURFACE9 lpd3d9Surface;
	LPDIRECT3DSURFACE9 lpd3d9OffscreenSurface;
	scrntype *lpd3d9Buffer;
	bool render_to_d3d9Buffer;
	bool use_d3d9;
	bool wait_vsync;
	
	// record video
	_TCHAR video_file_name[_MAX_PATH];
	int rec_video_fps;
	double rec_video_run_frames;
	double rec_video_frames;
	
	LPBITMAPINFO lpDibRec;
	PAVIFILE pAVIFile;
	PAVISTREAM pAVIStream;
	PAVISTREAM pAVICompressed;
	AVICOMPRESSOPTIONS opts;
	DWORD dwAVIFileSize;
	LONG lAVIFrames;
	
	HDC hdcDibRec;
	HBITMAP hBmpRec, hOldBmpRec;
	LPBYTE lpBufRec;
	scrntype* lpBmpRec;
	
	bool use_video_thread;
	HANDLE hVideoThread;
	video_thread_t video_thread_param;
	
	// ----------------------------------------
	// sound
	// ----------------------------------------
	void initialize_sound();
	void release_sound();
	void update_sound(int* extra_frames);
	
	int sound_rate, sound_samples;
	bool sound_ok, sound_started, now_mute;
	
	// direct sound
	LPDIRECTSOUND lpds;
	LPDIRECTSOUNDBUFFER lpdsb, lpdsp;
	bool first_half;
	
	// record sound
	_TCHAR sound_file_name[_MAX_PATH];
	FILEIO* rec;
	int rec_bytes;
	int rec_buffer_ptr;
	
#ifdef USE_DIRECT_SHOW
	// ----------------------------------------
	// direct show
	// ----------------------------------------
	void initialize_direct_show();
	void release_direct_show();
	void create_direct_show_dib_section();
	void release_direct_show_dib_section();
	
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
	bool bVirticalReversed;
	
	HDC hdcDibDShow;
	HBITMAP hBmpDShow, hOldBmpDShow;
	LPBYTE lpBufDShow;
	scrntype* lpBmpDShow;
	LPBITMAPINFO lpDibDShow;
	
	int direct_show_width, direct_show_height;
	bool direct_show_mute[2];
#ifdef USE_LASER_DISC
	double movie_frame_rate;
	int movie_sound_rate;
	bool now_movie_play, now_movie_pause;
#endif
#ifdef USE_VIDEO_CAPTURE
	void enum_capture_devs();
	bool connect_capture_dev(int index, bool pin);
	int cur_capture_dev_index;
	int num_capture_devs;
	_TCHAR capture_dev_name[MAX_CAPTURE_DEVS][256];
#endif
#endif
	
	// ----------------------------------------
	// media
	// ----------------------------------------
	typedef struct {
		_TCHAR path[_MAX_PATH];
		bool play;
		int offset;
		int wait_count;
	} media_status_t;
	
#ifdef USE_CART1
	media_status_t cart_status[MAX_CART];
#endif
#ifdef USE_FD1
	media_status_t disk_status[MAX_FD];
#endif
#ifdef USE_QD1
	media_status_t quickdisk_status[MAX_QD];
#endif
#ifdef USE_TAPE
	media_status_t tape_status;
#endif
#ifdef USE_LASER_DISC
	media_status_t laser_disc_status;
#endif
	
	void initialize_media();
	void update_media();
	void restore_media();
	
	void clear_media_status(media_status_t *status)
	{
		status->path[0] = _T('\0');
		status->wait_count = 0;
	}
	
	// ----------------------------------------
	// printer
	// ----------------------------------------
	void initialize_printer();
	void release_printer();
	void reset_printer();
	void update_printer();
	void open_printer_file();
	void close_printer_file();
	
	_TCHAR prn_file_name[MAX_PATH];
	FILEIO *prn_fio;
	int prn_data, prn_wait_frames;
	bool prn_strobe;
	
#ifdef USE_SOCKET
	// ----------------------------------------
	// socket
	// ----------------------------------------
	void initialize_socket();
	void release_socket();
	void update_socket();
	
	int soc[SOCKET_MAX];
	bool is_tcp[SOCKET_MAX];
	struct sockaddr_in udpaddr[SOCKET_MAX];
	int socket_delay[SOCKET_MAX];
	char recv_buffer[SOCKET_MAX][SOCKET_BUFFER_MAX];
	int recv_r_ptr[SOCKET_MAX], recv_w_ptr[SOCKET_MAX];
#endif
	
#ifdef USE_DEBUGGER
	// ----------------------------------------
	// debugger
	// ----------------------------------------
	void initialize_debugger();
	void release_debugger();
	HANDLE hDebuggerThread;
	debugger_thread_t debugger_thread_param;
#endif
	
#ifdef _DEBUG_LOG
	// ----------------------------------------
	// debug log
	// ----------------------------------------
	void initialize_debug_log();
	void release_debug_log();
	FILE* debug_log;
#endif
	
#ifdef USE_STATE
	// ----------------------------------------
	// state
	// ----------------------------------------
	void save_state_tmp(_TCHAR* file_path);
	bool load_state_tmp(_TCHAR* file_path);
#endif
	
	// ----------------------------------------
	// misc
	// ----------------------------------------
#ifdef USE_CPU_TYPE
	int cpu_type;
#endif
#ifdef USE_SOUND_DEVICE_TYPE
	int sound_device_type;
#endif
	_TCHAR app_path[_MAX_PATH];
	bool now_suspended;
	
public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
	EMU(HWND hwnd, HINSTANCE hinst);
	~EMU();
	
	_TCHAR* application_path()
	{
		return app_path;
	}
	_TCHAR* bios_path(_TCHAR* file_name);
	
	// ----------------------------------------
	// for windows
	// ----------------------------------------
	HWND main_window_handle;
	HINSTANCE instance_handle;
	
	// drive virtual machine
	int frame_interval();
	int run();
	void reset();
#ifdef USE_SPECIAL_RESET
	void special_reset();
#endif
#ifdef USE_POWER_OFF
	void notify_power_off();
#endif
	void suspend();
	
	// media
#ifdef USE_FD1
	typedef struct {
		_TCHAR name[18];
		int offset;
	} d88_bank_t;
	typedef struct {
		_TCHAR path[_MAX_PATH];
		d88_bank_t bank[MAX_D88_BANKS];
		int bank_num;
		int cur_bank;
	} d88_file_t;
	d88_file_t d88_file[MAX_FD];
#endif
	
	// user interface
#ifdef USE_CART1
	void open_cart(int drv, _TCHAR* file_path);
	void close_cart(int drv);
	bool cart_inserted(int drv);
#endif
#ifdef USE_FD1
	void open_disk(int drv, _TCHAR* file_path, int offset);
	void close_disk(int drv);
	bool disk_inserted(int drv);
#endif
#ifdef USE_QD1
	void open_quickdisk(int drv, _TCHAR* file_path);
	void close_quickdisk(int drv);
	bool quickdisk_inserted(int drv);
#endif
#ifdef USE_TAPE
	void play_tape(_TCHAR* file_path);
	void rec_tape(_TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
#endif
#ifdef USE_TAPE_BUTTON
	void push_play();
	void push_stop();
#endif
#ifdef USE_LASER_DISC
	void open_laser_disc(_TCHAR* file_path);
	void close_laser_disc();
	bool laser_disc_inserted();
#endif
#ifdef USE_BINARY_FILE1
	void load_binary(int drv, _TCHAR* file_path);
	void save_binary(int drv, _TCHAR* file_path);
#endif
	bool now_skip();
	
	void start_rec_sound();
	void stop_rec_sound();
	void restart_rec_sound();
	bool now_rec_sound;
	
	void capture_screen();
	bool start_rec_video(int fps);
	void stop_rec_video();
	void restart_rec_video();
	bool now_rec_video;
	
	void update_config();
	
#ifdef USE_STATE
	void save_state();
	void load_state();
#endif
	
	// input device
	void key_down(int code, bool repeat);
	void key_up(int code);
	void key_lost_focus()
	{
		lost_focus = true;
	}
#ifdef USE_BUTTON
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
	
	// screen
	int get_window_width(int mode);
	int get_window_height(int mode);
	void set_display_size(int width, int height, bool window_mode);
	int draw_screen();
	void update_screen(HDC hdc);
#ifdef USE_BITMAP
	void reload_bitmap()
	{
		first_invalidate = true;
	}
#endif
	
	// sound
	void mute_sound();
	
#ifdef USE_VIDEO_CAPTURE
	// video capture
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
#endif
	
#ifdef USE_SOCKET
	// socket
	int get_socket(int ch)
	{
		return soc[ch];
	}
	void socket_connected(int ch);
	void socket_disconnected(int ch);
	void send_data(int ch);
	void recv_data(int ch);
#endif
	
#ifdef USE_DEBUGGER
	// debugger
	void open_debugger(int cpu_index);
	void close_debugger();
	bool debugger_enabled(int cpu_index);
	bool now_debugging;
#endif
	
	// ----------------------------------------
	// for virtual machine
	// ----------------------------------------
	
	// power off
	void power_off()
	{
		PostMessage(main_window_handle, WM_CLOSE, 0, 0L);
	}
	
	// input device
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
	
	// screen
	void change_screen_size(int sw, int sh, int swa, int sha, int ww, int wh);
	scrntype* screen_buffer(int y);
#ifdef USE_CRT_FILTER
	bool screen_skip_line;
#endif
	
	// timer
	void get_host_time(cur_time_t* time);
	
	// printer
	void printer_out(uint8 value);
	void printer_strobe(bool value);
	
#ifdef USE_DIRECT_SHOW
	// direct show
	void get_direct_show_buffer();
	void mute_direct_show_dev(bool l, bool r);
	
#ifdef USE_LASER_DISC
	bool open_movie_file(_TCHAR* file_path);
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
#endif
#ifdef USE_VIDEO_CAPTURE
	void set_capture_dev_channel(int ch);
#endif
#endif
	
#ifdef USE_SOCKET
	// socket
	bool init_socket_tcp(int ch);
	bool init_socket_udp(int ch);
	bool connect_socket(int ch, uint32 ipaddr, int port);
	void disconnect_socket(int ch);
	bool listen_socket(int ch);
	void send_data_tcp(int ch);
	void send_data_udp(int ch, uint32 ipaddr, int port);
#endif
	
	// debug log
	void out_debug_log(const _TCHAR* format, ...);
	
	void out_message(const _TCHAR* format, ...);
	int message_count;
	_TCHAR message[1024];
};

#endif
