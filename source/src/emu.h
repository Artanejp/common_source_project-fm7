/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 emulation i/f ]
*/

#ifndef _EMU_H_
#define _EMU_H_


#if defined(_USE_QT)
# include <SDL.h>
# include "simd_types.h"
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
#define OSD_QT
#elif defined(_USE_SDL)
#define OSD_SDL
#elif defined(_WIN32)
#define OSD_WIN32
#else
// oops!
#endif


// OS dependent header files should be included in each osd.h
// Please do not include them in emu.h

#if defined(OSD_QT)
#include "qt/osd.h"
#elif defined(OSD_SDL)
#include "sdl/osd.h"
#elif defined(OSD_WIN32)
#include "win32/osd.h"
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
class OSD;

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

#if defined(OSD_QT)
class GLDrawClass;
class EmuThreadClass;
class DrawThreadClass;
#endif

class EMU
{
protected:
	VM* vm;
	OSD* osd;
private:
	_TCHAR app_path[_MAX_PATH];
	// debugger
#ifdef USE_DEBUGGER
	void initialize_debugger();
	void release_debugger();
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
#ifdef USE_PRINTER
	int printer_device_type;
#endif
	bool now_suspended;
	
	// input
#ifdef USE_AUTO_KEY
	FIFO* auto_key_buffer;
	int auto_key_phase, auto_key_shift;
	void initialize_auto_key();
	void release_auto_key();
	void update_auto_key();
#endif
#ifdef USE_JOYSTICK
	uint32_t joy_status[4];
	void update_joystick();
#endif
	
	// media
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
	media_status_t floppy_disk_status[MAX_FD];
#endif
#ifdef USE_QD1
	media_status_t quick_disk_status[MAX_QD];
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
	
	// state
#ifdef USE_STATE
	void save_state_tmp(const _TCHAR* file_path);
	bool load_state_tmp(const _TCHAR* file_path);
#endif

public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
#if defined(OSD_QT)
	EMU(class Ui_MainWindow *hwnd, GLDrawClass *hinst);
#elif defined(OSD_WIN32)
 	EMU(HWND hwnd, HINSTANCE hinst);
#else
	EMU();
#endif
	~EMU();

	// ----------------------------------------
	// for windows
	// ----------------------------------------
#ifdef OSD_QT
	// qt dependent
	EmuThreadClass *get_parent_handler();
	void set_parent_handler(EmuThreadClass *p, DrawThreadClass *q);
	VM *get_vm()
	{
		return vm;
	}
	OSD *get_osd()
	{
		return osd;
	}
	void set_host_cpus(int v);
	int get_host_cpus();
	void set_mouse_pointer(int x, int y);
	void set_mouse_button(int button);
	int get_mouse_button();
#endif
	
	// drive machine
	int get_frame_interval();
	bool is_frame_skippable();
	int run();

	void reset();
#ifdef USE_SPECIAL_RESET
	void special_reset();
#endif
#ifdef USE_NOTIFY_POWER_OFF
	void notify_power_off();
#endif
	void power_off();
	void suspend();
 	void lock_vm();
 	void unlock_vm();
	void force_unlock_vm();
	bool is_vm_locked();
   
	// input
#ifdef OSD_QT
	void key_modifiers(uint32_t mod);
#endif
	void key_down(int code, bool repeat);
	void key_up(int code);
	void key_lost_focus();
#ifdef ONE_BOARD_MICRO_COMPUTER
	void press_button(int num);
#endif
	void enable_mouse();
	void disable_mouse();
	void toggle_mouse();
	bool is_mouse_enabled();
#ifdef USE_AUTO_KEY
	void start_auto_key();
	void stop_auto_key();
	bool is_auto_key_running()
	{
		return (auto_key_phase != 0);
	}
	FIFO* get_auto_key_buffer()
	{
		return auto_key_buffer;
	}
#endif
	
	const uint8_t* get_key_buffer();
	const uint32_t* get_joy_buffer();
	const int* get_mouse_buffer();
	
	// screen
	int get_window_width(int mode);
	int get_window_height(int mode);
	void set_host_window_size(int window_width, int window_height, bool window_mode);
	void set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect);
	int get_vm_window_width();
	int get_vm_window_height();
	int get_vm_window_width_aspect();
	int get_vm_window_height_aspect();
#if defined(USE_MINIMUM_RENDERING)
	bool is_screen_changed();
#endif
	int draw_screen();
	scrntype_t* get_screen_buffer(int y);
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
	bool start_record_video(int fps);
	void stop_record_video();
	bool is_video_recording();
	
	// sound
	void mute_sound();
	void start_record_sound();
	void stop_record_sound();
	bool is_sound_recording();
	
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
	uint32_t get_cur_movie_frame();
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
	
#ifdef USE_PRINTER
	void create_bitmap(bitmap_t *bitmap, int width, int height);
	void release_bitmap(bitmap_t *bitmap);
	void create_font(font_t *font, const _TCHAR *family, int width, int height, int rotate, bool bold, bool italic);
	void release_font(font_t *font);
	void create_pen(pen_t *pen, int width, uint8_t r, uint8_t g, uint8_t b);
	void release_pen(pen_t *pen);
	void clear_bitmap(bitmap_t *bitmap, uint8_t r, uint8_t g, uint8_t b);
	int get_text_width(bitmap_t *bitmap, font_t *font, const char *text);
	void draw_text_to_bitmap(bitmap_t *bitmap, font_t *font, int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b);
	void draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen, int sx, int sy, int ex, int ey);
	void draw_rectangle_to_bitmap(bitmap_t *bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b);
	void draw_point_to_bitmap(bitmap_t *bitmap, int x, int y, uint8_t r, uint8_t g, uint8_t b);
	void stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y, int dest_width, int dest_height, bitmap_t *source, int source_x, int source_y, int source_width, int source_height);
	void write_bitmap_to_file(bitmap_t *bitmap, const _TCHAR *file_path);
#endif
	// socket
#ifdef USE_SOCKET
	int get_socket(int ch);
	void notify_socket_connected(int ch);
	void notify_socket_disconnected(int ch);
	bool initialize_socket_tcp(int ch);
	bool initialize_socket_udp(int ch);
	bool connect_socket(int ch, uint32_t ipaddr, int port);
	void disconnect_socket(int ch);
	bool listen_socket(int ch);
	void send_socket_data_tcp(int ch);
	void send_socket_data_udp(int ch, uint32_t ipaddr, int port);
	void send_socket_data(int ch);
	void recv_socket_data(int ch);
#endif
	
	// debugger
#ifdef USE_DEBUGGER
	void open_debugger(int cpu_index);
	void close_debugger();
	bool is_debugger_enabled(int cpu_index);
	bool now_debugging;
	debugger_thread_t debugger_thread_param;
#if defined(OSD_QT)
	SDL_Thread *debugger_thread_id;
	CSP_Debugger *hDebugger;
#elif defined(OSD_WIN32)
	HANDLE hDebuggerThread;
#else
	int debugger_thread_id;
#endif
#endif
	
	// debug log
	void out_debug_log(const _TCHAR* format, ...);
	void out_message(const _TCHAR* format, ...);
	int message_count;
	_TCHAR message[1024];
 	
	// misc
	void sleep(uint32_t ms);

	// debug log
#ifdef _DEBUG_LOG
	void initialize_debug_log();
	void release_debug_log();
	FILE* debug_log;
#endif
	
	// media
#ifdef USE_FD1
	struct {
		_TCHAR path[_MAX_PATH];
		_TCHAR disk_name[MAX_D88_BANKS][128];  // Convert to UTF8
 		int bank_num;
		int cur_bank;
	} d88_file[MAX_FD];
#endif

	// user interface
#ifdef USE_CART1
	void open_cart(int drv, const _TCHAR* file_path);
	void close_cart(int drv);
	bool is_cart_inserted(int drv);
#endif
#ifdef USE_FD1
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank);
	void close_floppy_disk(int drv);
	bool is_floppy_disk_inserted(int drv);
	void is_floppy_disk_protected(int drv, bool value);
	bool is_floppy_disk_protected(int drv);
#endif
#ifdef USE_QD1
	void open_quick_disk(int drv, const _TCHAR* file_path);
	void close_quick_disk(int drv);
	bool is_quick_disk_inserted(int drv);
#endif
#ifdef USE_TAPE
	void play_tape(const _TCHAR* file_path);
	void rec_tape(const _TCHAR* file_path);
	void close_tape();
	bool is_tape_inserted();
# ifndef TAPE_BINARY_ONLY
	bool is_tape_playing();
	bool is_tape_recording();
	int get_tape_position();
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
	bool is_laser_disc_inserted();
#endif
#ifdef USE_BINARY_FILE1
	void load_binary(int drv, const _TCHAR* file_path);
	void save_binary(int drv, const _TCHAR* file_path);
#endif
#ifdef USE_ACCESS_LAMP
	uint32_t get_access_lamp_status(void);
#endif	
#ifdef USE_LED_DEVICE
	uint32_t get_led_status(void);
#endif
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r);
#endif
	void update_config();
	// state
#ifdef USE_STATE
	void save_state();
	void load_state();
#endif
};
#endif // _EMU_H_

