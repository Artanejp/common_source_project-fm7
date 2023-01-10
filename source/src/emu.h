/*
	Skelton for retropc emulator

	Author : Takeda.Toshiya
	Date   : 2006.08.18 -

	[ win32 emulation i/f ]
*/

#ifndef _EMU_H_
#define _EMU_H_

#include "./emu_template.h"
#include "vm/vm.h"
// for debug
//#define _DEBUG_LOG
#ifdef _DEBUG_LOG
	// output fdc debug log
//	#define _FDC_DEBUG_LOG
	// output scsi debug log
//	#define _SCSI_DEBUG_LOG
	// output dma debug log
//	#define _DMA_DEBUG_LOG
	// output i/o debug log
//	#define _IO_DEBUG_LOG
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

#ifdef USE_FLOPPY_DISK
#define MAX_D88_BANKS 64
#endif
#ifdef USE_BUBBLE
#define MAX_B77_BANKS 16
#endif

class VM;
class VM_TEMPLATE;
class FIFO;
class FILEIO;
class CSP_Logger;

class EMU : public EMU_TEMPLATE
{
protected:
	VM* vm;
private:
	// debugger
#ifdef USE_DEBUGGER
	void initialize_debugger() override;
	void release_debugger() override;
#endif
	// input
#ifdef USE_AUTO_KEY
	void initialize_auto_key();
	void release_auto_key();
	int get_auto_key_code(int code);
	void set_auto_key_code(int code);
	void update_auto_key();
#endif
#ifdef USE_JOYSTICK
	void update_joystick();
#endif
	
	// media
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
	bool load_state_tmp(const _TCHAR* file_path);
#endif

public:
	// ----------------------------------------
	// initialize
	// ----------------------------------------
#if defined(OSD_QT)
	EMU(class Ui_MainWindow *hwnd, GLDrawClass *hinst, std::shared_ptr<CSP_Logger> p_logger, std::shared_ptr<USING_FLAGS> p);
#elif defined(OSD_WIN32)
	EMU(HWND hwnd, HINSTANCE hinst);
#else
	EMU();
#endif
	~EMU();

	// ----------------------------------------
	// for windows
	// ----------------------------------------
	VM_TEMPLATE *get_vm() override
	{
		return (VM_TEMPLATE *)vm;
	}
	OSD_BASE *get_osd() override
	{
		return osd;
	}
#ifdef OSD_QT
	// qt dependent
	EmuThreadClass *get_parent_handler() override;
	void set_parent_handler(EmuThreadClass *p, DrawThreadClass *q) override;
	void set_host_cpus(int v) override;
	int get_host_cpus() override;
#endif
	
	// drive machine
	double get_frame_rate() override;
	int get_frame_interval() override;
	bool is_frame_skippable() override;
	const bool is_use_state() override;
	int run() override;

	void reset() override;
#ifdef USE_SPECIAL_RESET
	void special_reset(int num) override;
#endif
#ifdef USE_NOTIFY_POWER_OFF
	void notify_power_off() override;
#endif
	void power_off() override;
	void suspend() override;
 	void lock_vm() override;
 	void unlock_vm() override;
	void force_unlock_vm() override;
	bool is_vm_locked() override;
   
	// input
	void key_down(int code, bool extended, bool repeat) override;
	void key_up(int code, bool extended) override;
	void key_char(char code) override;
#ifdef USE_KEY_LOCKED
	bool get_caps_locked() override;
	bool get_kana_locked() override;
#endif
	void key_lost_focus() override;
#ifdef ONE_BOARD_MICRO_COMPUTER
	void press_button(int num) override;
#endif
#ifdef USE_AUTO_KEY
	void set_auto_key_list(char *buf, int size) override;
	void set_auto_key_char(char code) override;
	void start_auto_key() override;
	void stop_auto_key() override;
	bool is_auto_key_running() override
	{
		return (auto_key_phase != 0);
	}
	FIFO* get_auto_key_buffer() override
	{
		return auto_key_buffer;
	}
#endif
	
	const uint8_t* get_key_buffer() override;
#ifdef USE_JOYSTICK
	// Joystick buffer should be with locking and sampling.
	const uint32_t* get_joy_buffer() override;
	void release_joy_buffer(const uint32_t* ptr) override;
#endif	
#ifdef USE_MOUSE
	// Mouse buffer should be with locking and sampling.
	const int32_t* get_mouse_buffer() override;
	const int32_t  get_mouse_button() override;
	void release_mouse_buffer(const int32_t* ptr) override;
	void enable_mouse() override;
	void disable_mouse() override;
	void toggle_mouse() override;
	bool is_mouse_enabled() override;
#endif	
	// screen
	double get_window_mode_power(int mode) override;
	int get_window_mode_width(int mode) override;
	int get_window_mode_height(int mode) override;
	void set_host_window_size(int window_width, int window_height, bool window_mode) override;
	void set_vm_screen_size(int screen_width, int screen_height, int window_width, int window_height, int window_width_aspect, int window_height_aspect) override;
	void set_vm_screen_lines(int lines) override;
	
	int get_vm_window_width() override;
	int get_vm_window_height() override;
	int get_vm_window_width_aspect() override;
	int get_vm_window_height_aspect()  override;
#if defined(USE_MINIMUM_RENDERING)
	bool is_screen_changed() override;
#endif
	int draw_screen() override;
	scrntype_t* get_screen_buffer(int y) override;
#ifdef USE_SCREEN_FILTER
	void screen_skip_line(bool skip_line) override;
#endif
#ifdef ONE_BOARD_MICRO_COMPUTER
	void get_invalidated_rect(int *left, int *top, int *right, int *bottom) override;
	void reload_bitmap() override;
#endif
#ifdef OSD_WIN32
	void invalidate_screen();
	void update_screen(HDC hdc);
#endif
	void capture_screen() override;
	bool start_record_video(int fps) override;
	void stop_record_video() override;
	bool is_video_recording() override;
	// sound
	int get_sound_rate() override
	{
		return sound_rate;
	}
	void mute_sound() override;
	void start_record_sound() override;
	void stop_record_sound() override;
	bool is_sound_recording() override;
	
	// video device
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	void get_video_buffer() override;
	void mute_video_dev(bool l, bool r) override;
#endif
#ifdef USE_MOVIE_PLAYER
	bool open_movie_file(const _TCHAR* file_path) override;
	void close_movie_file() override;
	void play_movie() override;
	void stop_movie() override;
	void pause_movie() override;
	double get_movie_frame_rate() override;
	int get_movie_sound_rate() override;
	void set_cur_movie_frame(int frame, bool relative) override;
	uint32_t get_cur_movie_frame() override;
#endif
#ifdef USE_VIDEO_CAPTURE
	int get_cur_capture_dev_index() override;
	int get_num_capture_devs() override;
	_TCHAR* get_capture_dev_name(int index) override;
	void open_capture_dev(int index, bool pin) override;
	void close_capture_dev() override;
	void show_capture_dev_filter() override;
	void show_capture_dev_pin() override;
	void show_capture_dev_source() override;
	void set_capture_dev_channel(int ch) override;
#endif
	
#ifdef USE_PRINTER
	void create_bitmap(bitmap_t *bitmap, int width, int height) override;
	void release_bitmap(bitmap_t *bitmap) override;
	void create_font(font_t *font, const _TCHAR *family, int width, int height, int rotate, bool bold, bool italic) override;
	void release_font(font_t *font) override;
	void create_pen(pen_t *pen, int width, uint8_t r, uint8_t g, uint8_t b)  override;
	void release_pen(pen_t *pen)  override;
	void clear_bitmap(bitmap_t *bitmap, uint8_t r, uint8_t g, uint8_t b)  override;
	int get_text_width(bitmap_t *bitmap, font_t *font, const char *text)  override;
	void draw_text_to_bitmap(bitmap_t *bitmap, font_t *font, int x, int y, const char *text, uint8_t r, uint8_t g, uint8_t b)  override;
	void draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen, int sx, int sy, int ex, int ey)  override;
	void draw_rectangle_to_bitmap(bitmap_t *bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b)  override;
	void draw_point_to_bitmap(bitmap_t *bitmap, int x, int y, uint8_t r, uint8_t g, uint8_t b)  override;
	void stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y, int dest_width, int dest_height, bitmap_t *source, int source_x, int source_y, int source_width, int source_height)  override;
	void write_bitmap_to_file(bitmap_t *bitmap, const _TCHAR *file_path)  override;
#endif
	// socket
#ifdef USE_SOCKET
	SOCKET get_socket(int ch)  override;
	void notify_socket_connected(int ch)  override;
	void notify_socket_disconnected(int ch)  override;
	bool initialize_socket_tcp(int ch)  override;
	bool initialize_socket_udp(int ch)  override;
	bool connect_socket(int ch, uint32_t ipaddr, int port)  override;
	void disconnect_socket(int ch)  override;
	bool listen_socket(int ch)  override;
	void send_socket_data_tcp(int ch)  override;
	void send_socket_data_udp(int ch, uint32_t ipaddr, int port)  override;
	void send_socket_data(int ch)  override;
	void recv_socket_data(int ch)  override;
#endif
	
	// debugger
#ifdef USE_DEBUGGER
	void open_debugger(int cpu_index) override;
	void close_debugger(int cpu_index) override;
	bool is_debugger_enabled(int cpu_index) override;
#endif
	void start_waiting_in_debugger() override;
	void finish_waiting_in_debugger() override;
	void process_waiting_in_debugger() override;
	
	// debug log
	void out_debug_log(const _TCHAR* format, ...)  override;
	void force_out_debug_log(const _TCHAR* format, ...)  override;
   
	void out_message(const _TCHAR* format, ...)  override;
 	
	// misc
	void sleep(uint32_t ms)  override;

	// debug log
#ifdef _DEBUG_LOG
	void initialize_debug_log()  override;
	void release_debug_log()  override;
#endif
	
	// media
#ifdef USE_FLOPPY_DISK
	bool create_blank_floppy_disk(const _TCHAR* file_path, uint8_t type)  override;
	void open_floppy_disk(int drv, const _TCHAR* file_path, int bank)  override;
	void close_floppy_disk(int drv)  override;
	bool is_floppy_disk_connected(int drv)  override;
	bool is_floppy_disk_inserted(int drv)  override;
	void is_floppy_disk_protected(int drv, bool value)  override;
	bool is_floppy_disk_protected(int drv)  override;
	uint32_t is_floppy_disk_accessed()  override;
	uint32_t floppy_disk_indicator_color() override;
#endif

	// user interface
#ifdef USE_CART
	void open_cart(int drv, const _TCHAR* file_path)  override;
	void close_cart(int drv)  override;
	bool is_cart_inserted(int drv)  override;
#endif
#ifdef USE_QUICK_DISK
	void open_quick_disk(int drv, const _TCHAR* file_path)  override;
	void close_quick_disk(int drv)  override;
	bool is_quick_disk_connected(int drv)  override;
	bool is_quick_disk_inserted(int drv)  override;
	uint32_t is_quick_disk_accessed()  override;
#endif
#ifdef USE_HARD_DISK
	bool create_blank_hard_disk(const _TCHAR* file_path, int sector_size, int sectors, int surfaces, int cylinders)  override;
	void open_hard_disk(int drv, const _TCHAR* file_path)  override;
	void close_hard_disk(int drv)  override;
	bool is_hard_disk_inserted(int drv)  override;
	uint32_t is_hard_disk_accessed()  override;
#endif
#ifdef USE_TAPE
	void play_tape(int drv, const _TCHAR* file_path)  override;
	void rec_tape(int drv, const _TCHAR* file_path)  override;
	void close_tape(int drv)  override;
	bool is_tape_inserted(int drv)  override;
	bool is_tape_playing(int drv)  override;
	bool is_tape_recording(int drv)  override;
	int get_tape_position(int drv)  override;
	const _TCHAR* get_tape_message(int drv)  override;
	void push_play(int drv)  override;
	void push_stop(int drv)  override;
	void push_fast_forward(int drv)  override;
	void push_fast_rewind(int drv)  override;
	void push_apss_forward(int drv)  override;
	void push_apss_rewind(int drv)  override;
#endif
#ifdef USE_COMPACT_DISC
	void open_compact_disc(int drv, const _TCHAR* file_path)  override;
	void close_compact_disc(int drv)  override;
	bool is_compact_disc_inserted(int drv)  override;
	uint32_t is_compact_disc_accessed()  override;
#endif
#ifdef USE_LASER_DISC
	void open_laser_disc(int drv, const _TCHAR* file_path)  override;
	void close_laser_disc(int drv)  override;
	bool is_laser_disc_inserted(int drv)  override;
	uint32_t is_laser_disc_accessed()  override;
#endif
#ifdef USE_BINARY_FILE
	void load_binary(int drv, const _TCHAR* file_path)  override;
	void save_binary(int drv, const _TCHAR* file_path)  override;
#endif
#ifdef USE_BUBBLE
	void open_bubble_casette(int drv, const _TCHAR* file_path, int bank)  override;
	void close_bubble_casette(int drv)  override;
	bool is_bubble_casette_inserted(int drv)  override;
	bool is_bubble_casette_protected(int drv)  override;
	void is_bubble_casette_protected(int drv, bool value)  override;
#endif
#ifdef USE_LED_DEVICE
	uint32_t get_led_status()  override;
#endif
#ifdef USE_SOUND_VOLUME
	void set_sound_device_volume(int ch, int decibel_l, int decibel_r)  override;
#endif
	void update_config()  override;
	// state
#ifdef USE_STATE
	void save_state(const _TCHAR* file_path)  override;
	void load_state(const _TCHAR* file_path)  override;
	const _TCHAR *state_file_path(int num) override;
#endif
};

#endif // _EMU_H_

