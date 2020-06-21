/*
	Skelton for retropc emulator

	Author : Kyuma.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2020.06.21 -

	[ interface for emulation i/f ]
*/
#pragma once

#include <stdio.h>
#include <assert.h>
#include "common.h"
#include "config.h"

//#include "vm/vm_template.h"
#if defined(_USE_QT)
#include <pthread.h>
#define OSD_QT
#elif defined(_USE_SDL)
#include <pthread.h>
#define OSD_SDL
#elif defined(_WIN32)
#define OSD_WIN32
#else
// oops!
#endif

#if defined(OSD_QT)
#include "qt/osd_base.h"
#endif

class VM;
class VM_TEMPLATE;
class EMU;
class EMU_TEMPLATE;
class OSD;
class FIFO;
class FILEIO;

#if defined(OSD_QT)
class CSP_Debugger;
class CSP_DebuggerThread;

class USING_FLAGS;
class GLDrawClass;
class EmuThreadClass;
class DrawThreadClass;
#endif

typedef struct {
	EMU *emu;
	OSD *osd;
	VM *vm;
	int cpu_index;
	bool running;
	bool request_terminate;
} debugger_thread_t;

class DLL_PREFIX EMU_TEMPLATE {
protected:
	OSD_BASE* osd;
private:
	uint8_t dummy_key_buffer[256];
	uint32_t dummy_joy_buffer[8];
	int dummy_mouse_buffer[4];
public:
	// ----------------------------------------
	// initialize
	// ---------------------------------------
#if defined(OSD_QT)
	EMU_TEMPLATE(class Ui_MainWindow *hwnd, GLDrawClass *hinst, USING_FLAGS *p)
#elif defined(OSD_WIN32)
 	EMU_TEMPLATE(HWND hwnd, HINSTANCE hinst)
#else
	EMU_TEMPLATE()
#endif
	{
		memset(dummy_key_buffer, 0x00, sizeof(dummy_key_buffer));
		memset(dummy_joy_buffer, 0x00, sizeof(dummy_joy_buffer));
		memset(dummy_mouse_buffer, 0x00, sizeof(dummy_mouse_buffer));
		memset(message, 0x00, sizeof(message));

		now_debugging = false;
		memset(&debugger_thread_param, 0x00, sizeof(debugger_thread_t));

#if defined(OSD_QT)
		debugger_thread_id = (pthread_t)0;
		hDebugger = NULL;
#elif defined(OSD_WIN32)
		hDebuggerThread = (HANDLE)0;
#else
		debugger_thread_id = 0;
#endif
		
		message_count = 0;
		debug_log = NULL;
	}
	virtual ~EMU_TEMPLATE() {}

	// ----------------------------------------
	// for windows
	// ----------------------------------------
	virtual VM_TEMPLATE *get_vm() {	return NULL; }
	virtual OSD_BASE *get_osd() { return NULL; }

#ifdef OSD_QT
	// qt dependent
	virtual EmuThreadClass *get_parent_handler() { return NULL; }
	virtual void set_parent_handler(EmuThreadClass *p, DrawThreadClass *q) {}
	virtual void set_host_cpus(int v) {}
	virtual int get_host_cpus() { return 1; }
#endif
	virtual double get_frame_rate() { return 59.94; }
	virtual int get_frame_interval() { return 1; }
	virtual bool is_frame_skippable() { return false; }
	virtual int run() { return 1; }

	virtual void reset() {}
	virtual void special_reset() {}
	virtual void notify_power_off() {}
	virtual void power_off() {}
	virtual void suspend() {}
	
	// around locking VM, these are useful for multi-threaded environment.
 	virtual void lock_vm() {}
 	virtual void unlock_vm() {}
	virtual void force_unlock_vm() {}
	virtual bool is_vm_locked() { return false; }
	
	// input
	// around Keyboard.
	virtual void key_down(int code, bool extended, bool repeat) {}
	virtual void key_up(int code, bool extended) {}
	virtual void key_char(char code) {}
	
	virtual bool get_caps_locked() { return false; }
	virtual bool get_kana_locked() { return false; }
	virtual void key_lost_focus() {}
	virtual void press_button(int num) {}

	// Auto Key
	virtual void set_auto_key_list(char *buf, int size) {}
	virtual void set_auto_key_char(char code) {}
	virtual void start_auto_key() {}
	virtual void stop_auto_key() {}
	virtual bool is_auto_key_running() { return false; }
	virtual FIFO* get_auto_key_buffer() { return NULL; }
	// Mouse
	virtual const uint8_t* get_key_buffer() { return dummy_key_buffer; }
	virtual const uint32_t* get_joy_buffer() { return dummy_joy_buffer; }
	virtual const int* get_mouse_buffer() { return dummy_mouse_buffer; }
	virtual void enable_mouse() {}
	virtual void disable_mouse() {}
	virtual void toggle_mouse() {}
	virtual bool is_mouse_enabled() { return false; }

	// screen
	virtual double get_window_mode_power(int mode) { return 1.0; }
	virtual int get_window_mode_width(int mode) { return 640; }
	virtual int get_window_mode_height(int mode) { return 200; }
	virtual void set_host_window_size(int window_width, int window_height,
									  bool window_mode) {}
	virtual void set_vm_screen_size(int screen_width, int screen_height,
									int window_width, int window_height,
									int window_width_aspect, int window_height_aspect) {}
	virtual void set_vm_screen_lines(int lines) {}
	virtual int get_vm_window_width() { return 640; }
	virtual int get_vm_window_height() { return 200; }
	virtual int get_vm_window_width_aspect() { return 640; }
	virtual int get_vm_window_height_aspect() { return 200; }
	virtual bool is_screen_changed() { return true; }
	virtual int draw_screen() { return 1; }
	virtual scrntype_t* get_screen_buffer(int y) { return NULL;}
	virtual void screen_skip_line(bool skip_line) {}
	
	// One-board-micro-computer
	virtual void get_invalidated_rect(int *left, int *top, int *right, int *bottom) {}
	virtual void reload_bitmap() {}

#ifdef OSD_WIN32
	virtual void invalidate_screen() {}
	virtual void update_screen(HDC hdc) {}
#endif
	// Screen capture and recording.Will update API.
	virtual void capture_screen() {}
	virtual bool start_record_video(int fps) { return false;}
	virtual void stop_record_video() {}
	virtual bool is_video_recording() { return false; }

	// sound
	virtual int get_sound_rate() { return 44100; }
	virtual void mute_sound() {}
	virtual void start_record_sound() {}
	virtual void stop_record_sound() {}
	virtual bool is_sound_recording() { return false; }
	
	// video device
	virtual void get_video_buffer() {}
	virtual void mute_video_dev(bool l, bool r) {}

	// Movie player.
	virtual bool open_movie_file(const _TCHAR* file_path) { return false; }
	virtual void close_movie_file() {}
	virtual void play_movie() {}
	virtual void stop_movie() {}
	virtual void pause_movie() {}
	virtual double get_movie_frame_rate() { return 29.97; }
	virtual int get_movie_sound_rate() { return 44100; }
	virtual void set_cur_movie_frame(int frame, bool relative) {}
	virtual uint32_t get_cur_movie_frame() { return 0; }

	// Capturing video.
	virtual int get_cur_capture_dev_index() { return 0; }
	virtual int get_num_capture_devs() { return 0; }
	virtual _TCHAR* get_capture_dev_name(int index) { return (_TCHAR*)_T("DUMMY"); }
	virtual void open_capture_dev(int index, bool pin) {}
	virtual void close_capture_dev() {}
	virtual void show_capture_dev_filter() {}
	virtual void show_capture_dev_pin() {}
	virtual void show_capture_dev_source() {}
	virtual void set_capture_dev_channel(int ch) {}

	// Printer
	virtual void create_bitmap(bitmap_t *bitmap, int width, int height) {}
	virtual void release_bitmap(bitmap_t *bitmap) {}
	virtual void create_font(font_t *font,
							 const _TCHAR *family,
							 int width, int height,
							 int rotate, bool bold, bool italic) {}
	virtual void release_font(font_t *font) {}
	virtual void create_pen(pen_t *pen, int width,
							uint8_t r, uint8_t g, uint8_t b) {}
	virtual void release_pen(pen_t *pen) {}
	virtual void clear_bitmap(bitmap_t *bitmap,
							  uint8_t r, uint8_t g, uint8_t b) {}
	virtual int get_text_width(bitmap_t *bitmap,
							   font_t *font, const char *text) { return 16; }
	virtual void draw_text_to_bitmap(bitmap_t *bitmap, font_t *font,
									 int x, int y, const char *text,
									 uint8_t r, uint8_t g, uint8_t b) {}
	virtual void draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen,
									 int sx, int sy, int ex, int ey) {}
	virtual void draw_rectangle_to_bitmap(bitmap_t *bitmap,
										  int x, int y,
										  int width, int height,
										  uint8_t r, uint8_t g, uint8_t b) {}
	virtual void draw_point_to_bitmap(bitmap_t *bitmap,
									  int x, int y,
									  uint8_t r, uint8_t g, uint8_t b) {}
	virtual void stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y,
								int dest_width, int dest_height,
								bitmap_t *source,
								int source_x, int source_y, int source_width,
								int source_height) {}
	virtual void write_bitmap_to_file(bitmap_t *bitmap, const _TCHAR *file_path) {}

	// socket
	virtual int get_socket(int ch) { return 0; }
	virtual void notify_socket_connected(int ch) {}
	virtual void notify_socket_disconnected(int ch) {}
	virtual bool initialize_socket_tcp(int ch) { return false; }
	virtual bool initialize_socket_udp(int ch) { return false; }
	
	virtual bool connect_socket(int ch, uint32_t ipaddr, int port) {return false;}
	virtual void disconnect_socket(int ch) {}
	
	virtual bool listen_socket(int ch) { return false; }
	virtual void send_socket_data_tcp(int ch) {}
	virtual void send_socket_data_udp(int ch, uint32_t ipaddr, int port) {}
	
	virtual void send_socket_data(int ch) {}
	virtual void recv_socket_data(int ch) {}

	// debugger
	virtual void open_debugger(int cpu_index) {}
	virtual void close_debugger() {}
	virtual bool is_debugger_enabled(int cpu_index) { return false; }
	bool now_debugging;
	debugger_thread_t debugger_thread_param;

#if defined(OSD_QT)
	pthread_t debugger_thread_id;
	CSP_Debugger *hDebugger;
#elif defined(OSD_WIN32)
	HANDLE hDebuggerThread;
#else
	int debugger_thread_id;
#endif

	virtual void start_waiting_in_debugger() {}
	virtual void finish_waiting_in_debugger() {}
	virtual void process_waiting_in_debugger() {}
	bool now_waiting_in_debugger;
	
	// debug log
	virtual void out_debug_log(const _TCHAR* format, ...) {}
	virtual void force_out_debug_log(const _TCHAR* format, ...) {}
   
	virtual void out_message(const _TCHAR* format, ...) {}
	int message_count;
	_TCHAR message[1024];

	// misc
	virtual void sleep(uint32_t ms) {}

	// debug log
	virtual void initialize_debug_log() {}
	virtual void release_debug_log() {}
	FILE* debug_log;

	// media
	// floppy disk
	virtual bool create_blank_floppy_disk(const _TCHAR* file_path, uint8_t type) { return false;}
	virtual void open_floppy_disk(int drv, const _TCHAR* file_path, int bank) {}
	virtual void close_floppy_disk(int drv) {}
	virtual bool is_floppy_disk_connected(int drv) { return false;}
	virtual bool is_floppy_disk_inserted(int drv) { return false;} 
	virtual void is_floppy_disk_protected(int drv, bool value) {}
	virtual bool is_floppy_disk_protected(int drv) { return false;}
	virtual uint32_t is_floppy_disk_accessed() { return 0x00000000; }

	// cartridge
	virtual void open_cart(int drv, const _TCHAR* file_path) {}
	virtual void close_cart(int drv) {}
	virtual bool is_cart_inserted(int drv) { return false; }

	// quick disk
	virtual void open_quick_disk(int drv, const _TCHAR* file_path) {}
	virtual void close_quick_disk(int drv) {}
	virtual bool is_quick_disk_connected(int drv) { return false; }
	virtual bool is_quick_disk_inserted(int drv) { return false; }
	virtual uint32_t is_quick_disk_accessed() { return 0x00000000; }

	// hard disk
	virtual bool create_blank_hard_disk(const _TCHAR* file_path,
										int sector_size, int sectors,
										int surfaces, int cylinders) { return false; }
	virtual void open_hard_disk(int drv, const _TCHAR* file_path) {}
	virtual void close_hard_disk(int drv) {}
	virtual bool is_hard_disk_inserted(int drv) { return false; }
	virtual uint32_t is_hard_disk_accessed() { return 0x00000000; }
	
	// tape (CMT)
	virtual void play_tape(int drv, const _TCHAR* file_path) {}
	virtual void rec_tape(int drv, const _TCHAR* file_path) {}
	virtual void close_tape(int drv) {}
	virtual bool is_tape_inserted(int drv) { return false; }
	virtual bool is_tape_playing(int drv) { return false; }
	virtual bool is_tape_recording(int drv) { return false; }
	virtual int get_tape_position(int drv) { return 0; }
	virtual const _TCHAR* get_tape_message(int drv) { return _T(" "); }
	virtual void push_play(int drv) {}
	virtual void push_stop(int drv) {}
	virtual void push_fast_forward(int drv) {}
	virtual void push_fast_rewind(int drv) {}
	virtual void push_apss_forward(int drv) {}
	virtual void push_apss_rewind(int drv) {}

	// compact disc (CD)
	virtual void open_compact_disc(int drv, const _TCHAR* file_path) {}
	virtual void close_compact_disc(int drv) {}
	virtual bool is_compact_disc_inserted(int drv)  { return false; }
	virtual uint32_t is_compact_disc_accessed() { return 0x00000000; }

	// laser disc (LD)
	virtual void open_laser_disc(int drv, const _TCHAR* file_path) {}
	virtual void close_laser_disc(int drv) {}
	virtual bool is_laser_disc_inserted(int drv)  { return false; }
	virtual uint32_t is_laser_disc_accessed() { return 0x00000000; }

	// binary file (memory image)
	virtual void load_binary(int drv, const _TCHAR* file_path) {}
	virtual void save_binary(int drv, const _TCHAR* file_path) {}

	// bubble casette
	virtual void open_bubble_casette(int drv, const _TCHAR* file_path, int bank) {}
	virtual void close_bubble_casette(int drv) {}
	virtual bool is_bubble_casette_inserted(int drv) { return false; }
	virtual bool is_bubble_casette_protected(int drv) { return false; }
	virtual void is_bubble_casette_protected(int drv, bool value) {}

	// LED device
	virtual uint32_t get_led_status() { return 0x00000000; }

	// sound volume
	virtual void set_sound_device_volume(int ch, int decibel_l, int decibel_r) {}

	// configure
	virtual void update_config() {}

	// state
	virtual void save_state(const _TCHAR* file_path) {}
	virtual void load_state(const _TCHAR* file_path) {}

#ifdef OSD_QT
	// New APIs
	virtual void load_sound_file(int id, const _TCHAR *name, int16_t **data, int *dst_size) {}
	virtual void free_sound_file(int id, int16_t **data) {}
#endif
};

