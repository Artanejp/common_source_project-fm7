/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

#ifndef _QT_OSD_BASE_H_
#define _QT_OSD_BASE_H_


#include <QList>
#include <QThread>
#include <QString>
#include <QImage>
#include <SDL.h>
//#include "simd_types.h"

//#include <ctime>
//#include <limits>
//#include <osd_base.h>

//#include "../vm/vm.h"
//#include "../emu.h"
#include "../config.h"
//#include "../fileio.h"
//#include "../fifo.h"
//#if !defined(Q_OS_WIN32)
//#include "qt_input.h"
//#endif
#define SOCKET_MAX 4
#define SOCKET_BUFFER_MAX 0x100000
#include "osd_types.h"

#define N_MAX_BUTTONS 128

#define WM_RESIZE  (WM_USER + 1)
#define WM_SOCKET0 (WM_USER + 2)
#define WM_SOCKET1 (WM_USER + 3)
#define WM_SOCKET2 (WM_USER + 4)
#define WM_SOCKET3 (WM_USER + 5)

//#ifdef USE_VIDEO_CAPTURE
#define MAX_CAPTURE_DEVS 8
//#endif

//#include "qt_main.h"

class GLDrawClass;
class EmuThreadClass;
class DrawThreadClass;
class Ui_MainWindow;
class EMU;
class VM_TEMPLATE;
class FIFO;
class FILEIO;
class CSP_KeyTables;
class USING_FLAGS;
class CSP_logger;
class QMutex;

QT_BEGIN_NAMESPACE

typedef struct {
	int id;
	const _TCHAR *name;
} device_node_t;

typedef struct {
	QString string;
	union {
		int64_t ivalue;
		double  fvalue;
	} v;
} supportedlist_t;

class DLL_PREFIX OSD_BASE : public QThread
{
	Q_OBJECT
protected:
	EmuThreadClass *parent_thread;
	sdl_snddata_t snddata;
	USING_FLAGS *using_flags;
	config_t *p_config;
	CSP_Logger *p_logger;

	QList<supportedlist_t> SupportedFeatures;
	
	bool __USE_AUTO_KEY;
	bool __USE_SHIFT_NUMPAD_KEY;
   
	_TCHAR app_path[_MAX_PATH];
	QElapsedTimer osd_timer;
	bool locked_vm;
	// console
	FILE *hStdIn, *hStdOut;
	QString console_cmd_str;
	bool osd_console_opened;
	// input
	void initialize_input();
	void release_input();
	void key_down_sub(int code, bool repeat);
	void key_up_sub(int code);
	CSP_KeyTables *key_table;
	
	scrntype_t *get_buffer(bitmap_t *p, int y);
	bool dinput_key_ok;
//	bool dinput_joy_ok;
	
	uint8_t keycode_conv[256];
	uint8_t key_status[256];	// windows key code mapping
	uint8_t key_dik_prev[256];
	uint8_t key_converted[256];
	bool key_shift_pressed, key_shift_released;


	uint32_t modkey_status;
	bool lost_focus;
	uint32_t joy_status[4];	// joystick #1, #2 (b0 = up, b1 = down, b2 = left, b3 = right, b4- = buttons
	int joy_num;
	uint32_t joy_mask[4];
	
	int32_t mouse_status[3];	// x, y, button (b0 = left, b1 = right)
	bool mouse_enabled;
	int mouse_ptrx;
	int mouse_ptry;
	int mouse_button;
	int mouse_oldx;
	int mouse_oldy;
	int delta_x;
	int delta_y;
	//Qt::CursorShape mouse_shape;
	
	QImage background_image;
	QImage button_images[N_MAX_BUTTONS];
	QImage rec_image_buffer;
	
	// printer
	
	// screen
	void initialize_screen();
	void release_screen();
	void initialize_screen_buffer(bitmap_t *buffer, int width, int height, int mode);
	void release_screen_buffer(bitmap_t *buffer);
	void rotate_screen_buffer(bitmap_t *source, bitmap_t *dest);
	
	void stretch_screen_buffer(bitmap_t *source, bitmap_t *dest);
	int add_video_frames();
	
	bitmap_t vm_screen_buffer;
	bitmap_t video_screen_buffer;
	bitmap_t* draw_screen_buffer;
	int vm_window_width, vm_window_height;
	int vm_window_width_aspect, vm_window_height_aspect;
	
	int host_window_width, host_window_height;
	bool host_window_mode;
	int base_window_width, base_window_height;
	int vm_screen_width, vm_screen_height;
	int draw_screen_width, draw_screen_height;
	int rec_video_nsec, rec_video_fps_nsec;
	
	_TCHAR video_file_name[_MAX_PATH];
	int rec_video_fps;
	
	uint64_t dwAVIFileSize;
	uint64_t lAVIFrames;

	rec_video_thread_param_t rec_video_thread_param;
	
	bool first_draw_screen;
	bool first_invalidate;
	bool self_invalidate;
	
	// sound
	void initialize_sound(int rate, int samples);
	void release_sound();
	static void audio_callback(void *udata, Uint8 *stream, int len);
	int sound_rate, sound_samples;
	bool sound_ok, sound_started, now_mute;
	bool sound_first_half;
	
	_TCHAR sound_file_name[_MAX_PATH];
	FILEIO* rec_sound_fio;
	int rec_sound_bytes;
	int rec_sound_buffer_ptr;
	
	int sound_buffer_size;
	int sound_data_len;
	int sound_data_pos;
	int sound_write_pos;
	bool sound_exit;
	bool sound_debug;
	bool sound_initialized;
	Sint16 *sound_buf_ptr;
	Uint8 snd_total_volume;
#if defined(USE_SDL2)   
	SDL_AudioDeviceID audio_dev_id;
#else
	int audio_dev_id;
#endif
	SDL_AudioSpec snd_spec_req, snd_spec_presented;
	
	// video device
	virtual void initialize_video();
	virtual void release_video();
  
	scrntype_t *mapped_screen_pointer;
	int mapped_screen_width;
	int mapped_screen_height;
	bool mapped_screen_status;
	bitmap_t dshow_screen_buffer;
	int direct_show_width, direct_show_height;
	bool direct_show_mute[2];

	double movie_frame_rate;
	int movie_sound_rate;

	void enum_capture_devs();
	bool connect_capture_dev(int index, bool pin);
	int cur_capture_dev_index;
	int num_capture_devs;
	_TCHAR capture_dev_name[MAX_CAPTURE_DEVS][256];

	_TCHAR prn_file_name[_MAX_PATH];
	FILEIO *prn_fio;
	int prn_data, prn_wait_frames;
	bool prn_strobe;

	// socket
	virtual void initialize_socket();
	virtual void release_socket();
	
	bool is_tcp[SOCKET_MAX];
	bool host_mode[SOCKET_MAX];
	int socket_delay[SOCKET_MAX];

	// wrapper
	int max_vm_nodes;
	QList<device_node_t> device_node_list;
	virtual void vm_draw_screen(void);
	virtual Sint16* create_sound(int *extra_frames);
	virtual bool get_use_socket(void);
	virtual bool get_use_auto_key(void);
	virtual bool get_dont_keeep_key_pressed(void);
	virtual bool get_one_board_micro_computer(void);
	virtual bool get_use_screen_rotate(void);
	virtual bool get_use_movie_player(void);
	virtual bool get_use_video_capture(void);
	virtual void vm_key_down(int code, bool flag);
	virtual void vm_key_up(int code);
	virtual void vm_reset(void);
	virtual void update_buttons(bool press_flag, bool release_flag);
	virtual int get_screen_width(void);
	virtual int get_screen_height(void);
	virtual int get_vm_buttons_code(int num);

	virtual void init_sound_files();
	virtual void release_sound_files();
public:
	OSD_BASE(USING_FLAGS *p, CSP_Logger *logger);
	~OSD_BASE();
	
	// common
	VM_TEMPLATE* vm;
	//EMU* emu;
	class Ui_MainWindow *main_window_handle;
	GLDrawClass *glv;
	QMutex *screen_mutex;
	QMutex *vm_mutex;
	
	int host_cpus;
	bool now_auto_key;
	
	virtual void initialize(int rate, int samples);
	virtual void release();
	virtual void power_off();
	void suspend();
	void restore();
	_TCHAR* application_path();
	_TCHAR* bios_path(const _TCHAR* file_name);
	void get_host_time(cur_time_t* time);
	void sleep(uint32_t ms);
	void create_date_file_name(_TCHAR *name, int length, const _TCHAR *extension);
	_TCHAR  *get_app_path(void);
	// common console
	void open_console(_TCHAR* title);
	void close_console();
	unsigned int get_console_code_page();
	bool is_console_active();
	void set_console_text_attribute(unsigned short attr);
	void write_console(_TCHAR* buffer, unsigned int length);
	int read_console_input(_TCHAR* buffer, int length);
	bool is_console_key_pressed(uint32_t ch);
	
	// common input
	void update_input();
	void key_down(int code, bool extended, bool repeat);
	void key_up(int code, bool extended);
	void key_down_native(int code, bool repeat);
	void key_up_native(int code);
	void key_lost_focus();
	void press_button(int num);

# if !defined(Q_OS_WIN) && !defined(Q_OS_CYGWIN)
	uint16_t GetAsyncKeyState(uint32_t vk);  // Win32 GetAsyncKeyState() wrappeer.
# endif
	void key_modifiers(uint32_t mod);
	void enable_mouse();
	void disable_mouse();
	void toggle_mouse();
	bool is_mouse_enabled();
	//QImage *getPseudoVramClass(void) { return pPseudoVram;}
	void set_mouse_pointer(int x, int y);
	void set_mouse_button(int button);
	int get_mouse_button();
	void modify_key_buffer(int code, uint8_t val);
	uint8_t* get_key_buffer();
	uint32_t* get_joy_buffer();
	int32_t* get_mouse_buffer();
	// common printer
	void reset_printer();
	void update_printer();
	void printer_out(uint8_t value);
	void printer_strobe(bool value);
	// printer
	void initialize_printer();
	void release_printer();
	void open_printer_file();
	void close_printer_file();
	
	// common screen
	int get_window_mode_width(int mode);
	int get_window_mode_height(int mode);
	double get_window_mode_power(int mode);
	void set_host_window_size(int window_width, int window_height, bool window_mode);
	void set_vm_screen_size(int width, int height, int width_aspect, int height_aspect, int window_width, int window_height);
	void set_vm_screen_lines(int lines); // 20170118
	int get_vm_window_width();
	int get_vm_window_height();
	int get_vm_window_width_aspect();
	int get_vm_window_height_aspect();
	scrntype_t* get_vm_screen_buffer(int y);
	//int draw_screen();
	//int no_draw_screen();
	void reload_bitmap();
	void capture_screen();
	bool start_record_video(int fps);
	void stop_record_video();
	void restart_record_video();
	void add_extra_frames(int extra_frames);
	bool now_record_video;
	bool screen_skip_line;
	// common sound
	void update_sound(int* extra_frames);
	void mute_sound();
	void stop_sound();
	void start_record_sound();
	void stop_record_sound();
	void restart_record_sound();
	bool now_record_sound;
	int get_sound_rate();
	// Wrapper : Sound
	virtual void load_sound_file(int id, const _TCHAR *name, int16_t **data, int *dst_size);
	virtual void free_sound_file(int id, int16_t **data);
	
	// common video device
	virtual void get_video_buffer();
	void mute_video_dev(bool l, bool r);
	virtual bool open_movie_file(const _TCHAR* file_path);
	virtual void close_movie_file();
	void play_movie();
	void stop_movie();
	void pause_movie();
	double get_movie_frame_rate();
	virtual int get_movie_sound_rate();
	void set_cur_movie_frame(int frame, bool relative);
	uint32_t get_cur_movie_frame();
	bool now_movie_play, now_movie_pause;
	int get_cur_capture_dev_index();
	int get_num_capture_devs();
	_TCHAR* get_capture_dev_name(int index);
	void open_capture_dev(int index, bool pin);
	void close_capture_dev();
	void show_capture_dev_filter();
	void show_capture_dev_pin();
	void show_capture_dev_source();
	void set_capture_dev_channel(int ch);
	
	// common printer
	void create_bitmap(bitmap_t *bitmap, int width, int height);
	void release_bitmap(bitmap_t *bitmap);
	void create_font(font_t *font, const _TCHAR *family, int width, int height, int rotate, bool bold, bool italic);
	void release_font(font_t *font);
	void create_pen(pen_t *pen, int width, uint8_t r, uint8_t g, uint8_t b);
	void release_pen(pen_t *pen);

	void clear_bitmap(bitmap_t *bitmap, uint8_t r, uint8_t g, uint8_t b);
	int get_text_width(bitmap_t *bitmap, font_t *font, const char *text);
	
	void draw_text_to_bitmap(bitmap_t *bitmap, font_t *font, int x, int y, const _TCHAR *text, uint8_t r, uint8_t g, uint8_t b);
	void draw_line_to_bitmap(bitmap_t *bitmap, pen_t *pen, int sx, int sy, int ex, int ey);
	void draw_rectangle_to_bitmap(bitmap_t *bitmap, int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b);
	void draw_point_to_bitmap(bitmap_t *bitmap, int x, int y, uint8_t r, uint8_t g, uint8_t b);

	void stretch_bitmap(bitmap_t *dest, int dest_x, int dest_y, int dest_width, int dest_height, bitmap_t *source, int source_x, int source_y, int source_width, int source_height);
	void write_bitmap_to_file(bitmap_t *bitmap, const _TCHAR *file_path);

	// common socket
	virtual int get_socket(int ch);
	virtual void notify_socket_connected(int ch);
	virtual void notify_socket_disconnected(int ch);
	virtual void update_socket();
	virtual bool initialize_socket_tcp(int ch);
	virtual bool initialize_socket_udp(int ch);
	virtual bool connect_socket(int ch, uint32_t ipaddr, int port);
	virtual void disconnect_socket(int ch);
	virtual bool listen_socket(int ch);
	virtual void send_socket_data_tcp(int ch);
	virtual void send_socket_data_udp(int ch, uint32_t ipaddr, int port);
	virtual void send_socket_data(int ch);
	virtual void recv_socket_data(int ch);

	// win32 dependent
	void update_screen();
	void set_parent_thread(EmuThreadClass *parent);
	EmuThreadClass *get_parent_handler();

	_TCHAR *console_input_string(void);
	void clear_console_input_string(void);
	// Wrapper
	virtual void lock_vm(void);
	virtual void unlock_vm(void);
	virtual void force_unlock_vm(void);
	virtual bool is_vm_locked(void);
	virtual void set_draw_thread(DrawThreadClass *handler);
	virtual QString get_vm_config_name(void);
	virtual double vm_frame_rate(void);
	virtual void reset_vm_node(void);
	virtual const _TCHAR *get_lib_common_vm_version() { return (const _TCHAR *)"\0"; }
	virtual const _TCHAR *get_lib_common_vm_git_version() { return (const _TCHAR *)"\0"; }
	virtual const _TCHAR *get_lib_osd_version() { return (const _TCHAR *)"\0"; }
	
	virtual void set_device_name(int id, char *name);
	
	virtual void set_vm_node(int id, const _TCHAR *name);
	virtual const _TCHAR *get_vm_node_name(int id);
	virtual int get_vm_node_size(void);
	
	// Get #define S to value.You may use inside of VM/ .
	virtual void set_features(void) {}
	void add_feature(const _TCHAR *key, double value);
	void add_feature(const _TCHAR *key, float value);
	void add_feature(const _TCHAR *key, int value = 1);
	void add_feature(const _TCHAR *key, int64_t value);
	void add_feature(const _TCHAR *key, uint32_t value);
	void add_feature(const _TCHAR *key, uint16_t value);
	void add_feature(const _TCHAR *key, uint8_t value);
	bool check_feature(const _TCHAR *key);
	double get_feature_double_value(const _TCHAR *key);
	int64_t get_feature_int_value(const _TCHAR *key);
	uint32_t get_feature_uint32_value(const _TCHAR *key);
	uint16_t get_feature_uint16_value(const _TCHAR *key);
	uint8_t get_feature_uint8_value(const _TCHAR *key);

	void debug_log(int level, const char *fmt, ...);
	void debug_log(int level, int domain_num, const char *fmt, ...);
	virtual void debug_log(int level, int domain_num, char *strbuf);

	USING_FLAGS *get_config_flags(void) { return using_flags; }

	// Special
	CSP_Logger *get_logger(void) { return p_logger; }
	
public slots:
	void do_write_inputdata(QString s);
	void do_set_input_string(QString s);
	void close_debugger_console();
	void do_close_debugger_thread();
	void do_assign_js_setting(int jsnum, int axis_idx, int assigned_value);
	void upload_bitmap(QImage *p);
	void set_buttons();
	void do_start_record_video();
	virtual void do_decode_movie(int frames);
	void do_video_movie_end(bool flag);
	void do_video_decoding_error(int num);
	virtual void do_run_movie_audio_callback(uint8_t *data, long len);
	int draw_screen();
	int no_draw_screen();
	void do_draw(bool flag);
	void do_set_screen_map_texture_address(scrntype_t *p, int width, int height);

signals:
	int sig_update_screen(bitmap_t *);
	int sig_save_screen(const char *);
	int sig_draw_frames(int);
	int sig_close_window(void);
	int sig_resize_vm_screen(QImage *, int, int);
	int sig_resize_vm_lines(int);
	int sig_put_string_debugger(QString);
	int sig_console_input_string(QString);
	int sig_enqueue_video(int, int, int, QImage *); 
	int sig_enqueue_audio(int16_t *data, int size);
	int sig_movie_set_width(int);
	int sig_movie_set_height(int);
	int sig_debugger_finished();
	int sig_req_encueue_video(int, int, int);
	int sig_save_as_movie(QString, int, int);
	int sig_stop_saving_movie();

	int sig_movie_play();
	int sig_movie_stop();
	int sig_movie_pause(bool);
	int sig_movie_seek_frame(bool, int);

	int sig_update_device_node_name(int id, const _TCHAR *name);
	int sig_enable_mouse(void);
	int sig_disable_mouse(void);
	int sig_close_console(void);

	int sig_move_mouse_to_center(void);
	
};
QT_END_NAMESPACE

#endif
