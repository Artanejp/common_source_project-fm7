/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 emulation i/f ]
*/

#ifndef _EMU_H_
#define _EMU_H_

// for debug
//#define _DEBUG_LOG
#ifdef _DEBUG_LOG
	// output fdc debug log
//	#define _FDC_DEBUG_LOG
	// output i/o debug log
//	#define _IO_DEBUG_LOG
#endif

#if defined(_USE_QT)
# include <SDL.h>
//# include "menuclasses.h"
//# include "mainwidget.h"
//# include "qt_gldraw.h"
//# include "emu_utils.h"
//# include "qt_main.h"
# include "simd_types.h"
// Wrapper of WIN32->*nix


#else // _USE_WIN32

#include <process.h>

#endif // _USE_WIN32

#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "config.h"
#include "vm/vm.h"
#if defined(USE_FD1)
#include "vm/disk.h"
#endif

#if defined(_USE_QT)
#include <QSemaphore>
#include <QMutex>
#include <QThread>
# if !defined(Q_OS_WIN32)
#include "qt_input.h"
# endif
#endif

#if defined(_USE_QT)
#define OSD_QT
#elif defined(_MSC_VER)
#define OSD_WIN32
#endif

#if defined(OSD_WIN32)
#include "win32/osd.h"
#elif defined(OSD_QT)
#include "qt/osd.h"
#elif defined(OSD_SDL)
#include "sdl/osd.h"
 #endif

#ifdef USE_FD1
#define MAX_D88_BANKS 64
#endif

#ifdef USE_SOCKET
#define SOCKET_MAX 4
#define SOCKET_BUFFER_MAX 0x100000
#endif


class EMU;
class OSD;
class FIFO;
class FILEIO;

#ifdef USE_DEBUGGER
typedef struct {
	OSD *osd;
	VM *vm;
	int cpu_index;
	bool running;
	bool request_terminate;
} debugger_thread_t;
class CSP_Debugger;
#endif

#ifdef __cplusplus
class EMU
{
protected:
	VM* vm;
	OSD* osd;
	int host_cpus;
private:
	_TCHAR app_path[_MAX_PATH];
	// ----------------------------------------
	// sound
	// ----------------------------------------
	void initialize_sound();
	void release_sound();
	void update_sound(int* extra_frames);
   
	// ----------------------------------------
	// media
	// ----------------------------------------
	typedef struct {
		_TCHAR path[_MAX_PATH];
		bool play;
		int bank;
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
#endif
#ifdef USE_LASER_DISC
	media_status_t laser_disc_status;
#endif

#ifdef USE_SOCKET
	int soc[SOCKET_MAX];
	bool is_tcp[SOCKET_MAX];
#if !defined(_USE_QT) 
	struct sockaddr_in udpaddr[SOCKET_MAX];
#endif
	int socket_delay[SOCKET_MAX];
	char recv_buffer[SOCKET_MAX][SOCKET_BUFFER_MAX];
	int recv_r_ptr[SOCKET_MAX], recv_w_ptr[SOCKET_MAX];
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
	// state
	// ----------------------------------------
#ifdef USE_STATE
	void save_state_tmp(const _TCHAR* file_path);
	bool load_state_tmp(const _TCHAR* file_path);
#endif

public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
#if defined(_USE_QT)
	EMU(class Ui_MainWindow *,  GLDrawClass *);
#else
	EMU(HWND hwnd, HINSTANCE hinst);
#endif
	~EMU();

#if defined(_USE_QT)
#else // M$ VC
	void LockVM(void) {
	}
	void UnlockVM(void) {
	}
#endif
	// ----------------------------------------
	// for windows
	// ----------------------------------------
#if defined(_USE_QT)
	class Ui_MainWindow *main_window_handle;
	GLDrawClass *instance_handle;
	EmuThreadClass *get_parent_handler(void);
	void set_parent_handler(EmuThreadClass *p) {
		osd->set_parent_thread(p);
	}
#ifdef USE_DEBUGGER
    debugger_thread_t debugger_thread_param;
	CSP_Debugger *hDebugger;
#endif   
	VM *getVM(void) {
		return vm;
	}
	void setMousePointer(int x, int y) {
		osd->setMousePointer(x, y);
	}
	void setMouseButton(int button) {
		osd->setMouseButton(button);
	}
	int getMouseButton() {
		return osd->getMouseButton();
	}
	void LockVM(void) {
		//if(host_cpus > 1) VMSemaphore->lock();
	}
	void UnlockVM(void) {
		//if(host_cpus > 1) VMSemaphore->unlock();
	}
	void SetHostCpus(int v) {
		if(v <= 0) v = 1;
		host_cpus = v;
	}
	int GetHostCpus() {
		return host_cpus;
	}
	//QThread *hDebuggerThread;
#else
	HWND main_window_handle;
	HINSTANCE instance_handle;
	bool vista_or_later;
#endif	
	// drive machine
	int frame_interval();
	int run();
	bool now_skip();
	void reset();
#ifdef USE_SPECIAL_RESET
	void special_reset();
#endif
#ifdef USE_NOTIFY_POWER_OFF
	void notify_power_off();
#endif
	void power_off();
	void suspend();
	// input
	void key_down(int code, bool repeat);
	void key_up(int code);
	void key_lost_focus();
#ifdef ONE_BOARD_MICRO_COMPUTER
	void press_button(int num);
#endif
	void enable_mouse();
	void disenable_mouse();
	void toggle_mouse();
	bool get_mouse_enabled();
#ifdef USE_AUTO_KEY
	void start_auto_key();
	void stop_auto_key();
	bool now_auto_key();
#endif
	
	uint8* key_buffer();
	uint32* joy_buffer();
	int* mouse_buffer();
	
	// screen
	int get_window_width(int mode);
	int get_window_height(int mode);
	void set_window_size(int width, int height, bool window_mode);
	void set_vm_screen_size(int sw, int sh, int swa, int sha, int ww, int wh);
	int draw_screen();
	scrntype* screen_buffer(int y);
#ifdef USE_CRT_FILTER
	void screen_skip_line(bool skip_line);
#endif
#ifdef ONE_BOARD_MICRO_COMPUTER
	void reload_bitmap();
#endif
#ifdef OSD_WIN32
	void update_screen(HDC hdc);
#endif
	void capture_screen();
	bool start_rec_video(int fps);
	void stop_rec_video();
	bool now_rec_video();
	
	// sound
	void mute_sound();
	void start_rec_sound();
	void stop_rec_sound();
	bool now_rec_sound();
	
	// video device
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
	double get_movie_frame_rate();
	int get_movie_sound_rate();
	void set_cur_movie_frame(int frame, bool relative);
	uint32 get_cur_movie_frame();
#endif
#ifdef USE_VIDEO_CAPTURE
	int get_cur_capture_dev_index();
	int get_num_capture_devs();
	_TCHAR* get_capture_dev_name(int index);
	void open_capture_dev(int index, bool pin);
	void close_capture_dev();
	void show_capture_dev_filter();
	void show_capture_dev_pin();
	void show_capture_dev_source();
	void set_capture_dev_channel(int ch);
#endif
	
	// printer
	void printer_out(uint8 value);
	void printer_strobe(bool value);
	// socket
#ifdef USE_SOCKET
	int get_socket(int ch);
	void socket_connected(int ch);
	void socket_disconnected(int ch);
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
	// debugger
#ifdef USE_DEBUGGER
	void open_debugger(int cpu_index);
	void close_debugger();
	bool debugger_enabled(int cpu_index);
	bool now_debugging;
#endif
	// debug log
	void out_debug_log(const _TCHAR* format, ...);
	void out_message(const _TCHAR* format, ...);
	int message_count;
	_TCHAR message[1024];
 	
	// misc
	_TCHAR* application_path();
	_TCHAR* bios_path(const _TCHAR* file_name);
	void sleep(uint32 ms);
	void get_host_time(cur_time_t* time);
#if defined(USE_MINIMUM_RENDERING)
	bool screen_changed() {
		return vm->screen_changed();
	}
#endif
	
	// debug log
#ifdef _DEBUG_LOG
	void initialize_debug_log();
	void release_debug_log();
	FILE* debug_log;
#endif
	
	// misc
	int sound_rate, sound_samples;
#ifdef USE_CPU_TYPE
	int cpu_type;
#endif
#ifdef USE_SOUND_DEVICE_TYPE
	int sound_device_type;
#endif
	bool now_suspended;
	
	// media
#ifdef USE_FD1
	struct {
		_TCHAR path[_MAX_PATH];
		_TCHAR disk_name[MAX_D88_BANKS][128];  // Convert to UTF8
 		int bank_num;
		int cur_bank;
	} d88_file[MAX_FD];
#endif
	int get_access_lamp(void);
#if defined(_USE_QT)
	void key_mod(uint32 mod) {
		osd->key_mod(mod);
	}
#endif	
	// user interface
#ifdef USE_CART1
	void open_cart(int drv, const _TCHAR* file_path);
	void close_cart(int drv);
	bool cart_inserted(int drv);
#endif
#ifdef USE_FD1
	void open_disk(int drv, const _TCHAR* file_path, int bank);
	void close_disk(int drv);
	bool disk_inserted(int drv);
	void set_disk_protected(int drv, bool value);
	bool get_disk_protected(int drv);
#endif
#ifdef USE_QD1
	void open_quickdisk(int drv, const _TCHAR* file_path);
	void close_quickdisk(int drv);
	bool quickdisk_inserted(int drv);
#endif
#ifdef USE_TAPE
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool tape_inserted();
# ifndef TAPE_BINARY_ONLY
	bool tape_playing();
	bool tape_recording();
	int tape_position();
# endif
# ifdef USE_TAPE_BUTTON
	void push_play();
	void push_stop();
	void push_fast_forward();
	void push_fast_rewind();
	void push_apss_forward();
	void push_apss_rewind();
# endif
#endif
#ifdef USE_LASER_DISC
	void open_laser_disc(const _TCHAR* file_path);
	void close_laser_disc();
	bool laser_disc_inserted();
#endif
#ifdef USE_BINARY_FILE1
	void load_binary(int drv, const _TCHAR* file_path);
	void save_binary(int drv, const _TCHAR* file_path);
#endif
#ifdef SUPPORT_DUMMY_DEVICE_LED
	uint32 get_led_status(void);
#endif
#if defined(USE_DIG_RESOLUTION)
	void get_screen_resolution(int *w, int *h);
#endif
	void update_config();
	// state
#ifdef USE_STATE
	void save_state();
	void load_state();
#endif
};
#endif // _EMU_H_

