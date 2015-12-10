/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

#ifndef _QT_OSD_H_
#define _QT_OSD_H_


#include <QWidget>
#include <QThread>
#include <QMutex>
#include <SDL.h>

#include "../vm/vm.h"
//#include "../emu.h"
#include "../config.h"
#include "../fileio.h"
#include "../fifo.h"

typedef struct {
   Sint16 **pSoundBuf;
   int *uBufSize;
   int *nSndWritePos;
   int *nSndDataLen;
   SDL_sem **pSndApplySem;
   Uint8 *iTotalVolume;
   bool *bSndExit;
   bool *bSoundDebug;
} sdl_snddata_t;



#if 0 // TODO
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
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

// check memory leaks
#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>
//#define malloc(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
//#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

typedef struct screen_buffer_s {
	int width, height;
	//HBITMAP hBmp, hOldBmp;
	//LPBYTE lpBuf;
	scrntype* lpBuf;
	QImage pImage;
} screen_buffer_t;

typedef struct {
	//PAVISTREAM pAVICompressed;
	scrntype* lpBmp;
	//LPBITMAPINFOHEADER pbmInfoHeader;
	DWORD dwAVIFileSize;
	UINT64 lAVIFrames;
	int frames;
	int result;
} rec_video_thread_param_t;

class GLDrawClass;
class EmuThreadClass;
class Ui_MainWindow;
class EMU;
class VM;

QT_BEGIN_NAMESPACE
class OSD : public QThread
{
	Q_OBJECT
protected:
//	VM* vm;
//	EMU* emu;
	EmuThreadClass *parent_thread;
	QMutex *VMMutex;
	_TCHAR auto_key_str[2048];
	sdl_snddata_t snddata;
	private:
	_TCHAR app_path[_MAX_PATH];
	
	// console
	FILE *hStdIn, *hStdOut;

	// input
	void initialize_input();
	void release_input();
	void key_down_sub(int code, bool repeat);
	void key_up_sub(int code);
	scrntype *get_buffer(screen_buffer_t *p, int y);
	bool dinput_key_ok;
//	bool dinput_joy_ok;
	
	uint8 keycode_conv[256];
	uint8 key_status[256];	// windows key code mapping
	uint8 key_dik_prev[256];
#ifdef USE_SHIFT_NUMPAD_KEY
	uint8 key_converted[256];
	bool key_shift_pressed, key_shift_released;
#endif
	uint32_t modkey_status;
	bool lost_focus;
	
	uint32 joy_status[2];	// joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4- = buttons
	int joy_num;
	uint32 joy_mask[2];
	
	int mouse_status[3];	// x, y, button (b0 = left, b1 = right)
	bool mouse_enabled;
	int mouse_ptrx;
	int mouse_ptry;
	int mouse_button;
	int mouse_oldx;
	int mouse_oldy;
	Qt::CursorShape mouse_shape;
	
#ifdef USE_AUTO_KEY
	FIFO* autokey_buffer;
	int autokey_phase, autokey_shift;
	int autokey_table[256];
#endif
	
	// printer
	
	// screen
	void initialize_screen();
	void release_screen();
	void initialize_screen_buffer(screen_buffer_t *buffer, int width, int height, int mode);
	void release_screen_buffer(screen_buffer_t *buffer);
#ifdef USE_CRT_FILTER
	void apply_crt_fileter_to_screen_buffer(screen_buffer_t *source, screen_buffer_t *dest);
	void apply_crt_filter_x3_y3(screen_buffer_t *source, screen_buffer_t *dest);
	void apply_crt_filter_x3_y2(screen_buffer_t *source, screen_buffer_t *dest);
	void apply_crt_filter_x2_y3(screen_buffer_t *source, screen_buffer_t *dest);
	void apply_crt_filter_x2_y2(screen_buffer_t *source, screen_buffer_t *dest);
	void apply_crt_filter_x1_y1(screen_buffer_t *source, screen_buffer_t *dest);
#endif
#ifdef USE_SCREEN_ROTATE
	void rotate_screen_buffer(screen_buffer_t *source, screen_buffer_t *dest);
#endif
	void stretch_screen_buffer(screen_buffer_t *source, screen_buffer_t *dest);
	int add_video_frames();
	
	screen_buffer_t vm_screen_buffer;
	screen_buffer_t video_screen_buffer;
#ifdef USE_CRT_FILTER
	screen_buffer_t filtered_screen_buffer;
#endif	
	screen_buffer_t* draw_screen_buffer;
	
	int host_window_width, host_window_height;
	bool host_window_mode;
	int base_window_width, base_window_height;
	int vm_screen_width, vm_screen_height, vm_screen_width_aspect, vm_screen_height_aspect;
	int draw_screen_width, draw_screen_height;
	
	
	_TCHAR video_file_name[_MAX_PATH];
	int rec_video_fps;
	double rec_video_run_frames;
	double rec_video_frames;
	
	//LPBITMAPINFO lpDibRec;
	//PAVIFILE pAVIFile;
	//PAVISTREAM pAVIStream;
	//PAVISTREAM pAVICompressed;
	//AVICOMPRESSOPTIONS AVIOpts;
	DWORD dwAVIFileSize;
	UINT64 lAVIFrames;
	//HANDLE hVideoThread;
	rec_video_thread_param_t rec_video_thread_param;
	
	bool first_draw_screen;
	bool first_invalidate;
	bool self_invalidate;
	
	// sound
	void initialize_sound(int rate, int samples);
	void release_sound();
	
	int sound_rate, sound_samples;
	bool sound_ok, sound_started, now_mute;
	bool sound_first_half;
	
	_TCHAR sound_file_name[_MAX_PATH];
	FILEIO* rec_sound_fio;
	int rec_sound_bytes;
	int rec_sound_buffer_ptr;
	
	
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	// video device
	void initialize_video();
	void release_video();
	
	//IGraphBuilder *pGraphBuilder;
	//IBaseFilter *pVideoBaseFilter;
	//IBaseFilter *pCaptureBaseFilter;
	//ICaptureGraphBuilder2 *pCaptureGraphBuilder2;
	//ISampleGrabber *pVideoSampleGrabber;
	//IBaseFilter *pSoundBaseFilter;
	//ISampleGrabber *pSoundSampleGrabber;
	//CMySampleGrabberCB *pSoundCallBack;
	//IMediaControl *pMediaControl;
	//IMediaSeeking *pMediaSeeking;
	//IMediaPosition *pMediaPosition;
	//IVideoWindow *pVideoWindow;
	//IBasicVideo *pBasicVideo;
	//IBasicAudio *pBasicAudio;
	//bool bTimeFormatFrame;
	//bool bVerticalReversed;
	
	screen_buffer_t dshow_screen_buffer;
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
	_TCHAR prn_file_name[_MAX_PATH];
	FILEIO *prn_fio;
	int prn_data, prn_wait_frames;
	bool prn_strobe;

	// socket
#ifdef USE_SOCKET
	void initialize_socket();
	void release_socket();
	
	int soc[SOCKET_MAX];
	bool is_tcp[SOCKET_MAX];
	//struct sockaddr_in udpaddr[SOCKET_MAX];
	int socket_delay[SOCKET_MAX];
	char recv_buffer[SOCKET_MAX][SOCKET_BUFFER_MAX];
	int recv_r_ptr[SOCKET_MAX], recv_w_ptr[SOCKET_MAX];
#endif
	
public:
	OSD();
	~OSD();
	
	// common
	VM* vm;
	//EMU* emu;
	class Ui_MainWindow *main_window_handle;
	GLDrawClass *glv;

	void initialize(int rate, int samples);
	void release();
	void power_off();
	void suspend();
	void restore();
	_TCHAR* application_path()
	{
		return app_path;
	}
	_TCHAR* bios_path(const _TCHAR* file_name);
	void get_host_time(cur_time_t* time);
	void sleep(uint32 ms);
	void create_date_file_name(_TCHAR *name, int length, _TCHAR *extension);
	
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
# if !defined(Q_OS_WIN) && !defined(Q_OS_CYGWIN)
	uint16_t GetAsyncKeyState(uint32_t vk);  // Win32 GetAsyncKeyState() wrappeer.
# endif
	void key_mod(uint32 mod) {
		modkey_status = mod;
	}
	void enable_mouse();
	void disenable_mouse();
	void toggle_mouse();
	bool get_mouse_enabled()
	{
		return mouse_enabled;
	}
        //QImage *getPseudoVramClass(void) { return pPseudoVram;}
	void setMousePointer(int x, int y) {
		mouse_ptrx = x;
		mouse_ptry = y;
	}
	void setMouseButton(int button) {
		mouse_button = button;
	}
	int getMouseButton() {
		return mouse_button;
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
	
	// common printer
	void reset_printer() {
		close_printer_file();
		prn_data = -1;
		prn_strobe = false;
	}
	void update_printer() {
		if(prn_fio->IsOpened() && --prn_wait_frames == 0) {
			close_printer_file();
		}
	}
	void printer_out(uint8 value) {
		prn_data = value;
	}
	void printer_strobe(bool value) {
		bool falling = (prn_strobe && !value);
		prn_strobe = value;
	
		if(falling) {
			if(!prn_fio->IsOpened()) {
				if(prn_data == -1) {
					return;
				}
				open_printer_file();
			}
			prn_fio->Fputc(prn_data);
			// wait 10sec
#ifdef SUPPORT_VARIABLE_TIMING
			prn_wait_frames = (int)(vm->frame_rate() * 10.0 + 0.5);
#else
			prn_wait_frames = (int)(FRAMES_PER_SEC * 10.0 + 0.5);
#endif
		}
	}
	// printer
	void initialize_printer();
	void release_printer();
	void open_printer_file() {
		create_date_file_name(prn_file_name, _MAX_PATH, _T("txt"));
		prn_fio->Fopen(bios_path(prn_file_name), FILEIO_WRITE_BINARY);
	}

	void close_printer_file() {
		if(prn_fio->IsOpened()) {
			// remove if the file size is less than 2 bytes
			bool remove = (prn_fio->Ftell() < 2);
			prn_fio->Fclose();
			if(remove) {
				FILEIO::RemoveFile(bios_path(prn_file_name));
			}
		}
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
	
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	// common video device
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
	
#ifdef USE_SOCKET
	// common socket
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
	void update_screen();
	void set_parent_thread(EmuThreadClass *parent);
	void lock_vm(void){
		VMMutex->lock();
	}
	void unlock_vm(void){
		VMMutex->unlock();
	}
public slots:
#ifdef USE_AUTO_KEY
	void set_auto_key_string(QByteArray);
#endif	
signals:
	int sig_update_screen(screen_buffer_t *);
	int sig_save_screen(const char *);
	int sig_close_window(void);
	int sig_resize_vm_screen(QImage *, int, int);
};
QT_END_NAMESPACE

#endif
