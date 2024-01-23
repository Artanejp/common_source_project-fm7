/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

#ifndef _QT_OSD_BASE_H_
#define _QT_OSD_BASE_H_


#include <QList>
#include <QObject>
#include <QThread>
#include <QString>
#include <QStringList>
#include <QImage>

#include <SDL.h>

#include <mutex>
#include <string>
#include <list>
#include <memory>
#include <atomic>

#define SOCKET_MAX 4
#define SOCKET_BUFFER_MAX 0x100000
#include "../config.h"
#include "osd_types.h"
// For UIs
#include "osdcall_types.h"

#define N_MAX_BUTTONS 128

#define WM_RESIZE  (WM_USER + 1)
#define WM_SOCKET0 (WM_USER + 2)
#define WM_SOCKET1 (WM_USER + 3)
#define WM_SOCKET2 (WM_USER + 4)
#define WM_SOCKET3 (WM_USER + 5)

// osd common

#define OSD_CONSOLE_BLUE       1 // text color contains blue
#define OSD_CONSOLE_GREEN      2 // text color contains green
#define OSD_CONSOLE_RED                4 // text color contains red
#define OSD_CONSOLE_INTENSITY  8 // text color is intensified

//#ifdef USE_VIDEO_CAPTURE
#define MAX_CAPTURE_DEVS 8
//#endif

//#include "qt_main.h"

enum {
	SAMPLE_TYPE_SINT8 = 0,
	SAMPLE_TYPE_UINT8,
	SAMPLE_TYPE_SINT16_BE,
	SAMPLE_TYPE_SINT16_LE,
	SAMPLE_TYPE_UINT16_BE,
	SAMPLE_TYPE_UINT16_LE,
	SAMPLE_TYPE_SINT32_BE,
	SAMPLE_TYPE_SINT32_LE,
	SAMPLE_TYPE_UINT32_BE,
	SAMPLE_TYPE_UINT32_LE,
	SAMPLE_TYPE_FLOAT_BE,
	SAMPLE_TYPE_FLOAT_LE,
};

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

class QOpenGLContext;
namespace SOUND_MODULE {
	namespace OUTPUT {
		class M_BASE;
	}
}

QT_BEGIN_NAMESPACE

#define MAX_SOUND_CAPTURE_DEVICES 8
#define MAX_CAPTURE_SOUNDS 16
typedef struct {
	int id;
	const _TCHAR *name;
} device_node_t;

typedef struct {
	QString string;
	union {
		int64_t ivalue;
		uint64_t uvalue;
		double  fvalue;
	} v;
} supportedlist_t;


typedef struct {
	SDL_AudioFormat format;
	int buffer_size;
	int readlen;
	int writelen;
	int readpos;
	int writepos;
	uint8_t* read_buffer_ptr;
} osd_snddata_capture_t;

typedef struct {
	SDL_AudioFormat format;
	int sample_rate;
	int channels;
	int buffer_samples;
	int silence;
	int size;
	SDL_AudioCallback callback;
	osd_snddata_capture_t userdata;
} osd_snd_capture_dev_desc_t;

typedef struct {
	int physical_dev;
	SDL_AudioFormat  read_format;
	int read_rate;
	int read_channels;
	int read_samples;
	int read_silence;
	int read_size;
	SDL_AudioCallback read_callback;
	void *read_userdata;
	// For output
	int sample_type; // ToDo : ENUM
	int rate;
	int channels;
	int samples;
	int write_size;
	int write_pos;
	int read_pos;
	int read_data_len;
	int read_buffer_len;

	uint8_t *read_buffer_ptr;
	uint8_t *out_buffer;
} osd_snd_capture_desc_t;



class SOUND_BUFFER_QT;

class DLL_PREFIX OSD_BASE : public  QObject
{
	Q_OBJECT
private:
	/* Note: Below are new sound driver. */
	std::shared_ptr<SOUND_MODULE::OUTPUT::M_BASE> m_sound_driver;
	int64_t elapsed_us_before_rendered;
	// Count half
	uint32_t     m_sound_period;
	// Count factor; this multiplies by 65536;
	uint32_t     m_sound_samples_count;
	uint32_t     m_sound_samples_factor;

protected:
	EmuThreadClass		*parent_thread;
	QThread				*m_sound_thread;
	std::shared_ptr<USING_FLAGS>			using_flags;
	config_t			*p_config;
	std::shared_ptr<CSP_Logger> p_logger;

	QOpenGLContext *glContext;
	bool is_glcontext_shared;

	QList<supportedlist_t> SupportedFeatures;

	bool __USE_AUTO_KEY;

	_TCHAR app_path[_MAX_PATH];
	QElapsedTimer osd_timer;
	bool locked_vm;
	// console
	virtual void initialize_console();
	virtual void release_console();

	FILE *hStdIn, *hStdOut;
	QString console_cmd_str;

	bool use_telnet;
	std::atomic<bool> telnet_closed;
	std::atomic<int> console_count;
	// input
	void initialize_input();
	void release_input();
	void key_down_sub(int code, bool repeat);
	void key_up_sub(int code);
	CSP_KeyTables *key_table;

	bool dinput_key_ok;
//	bool dinput_joy_ok;

	uint8_t keycode_conv[256];
	uint8_t key_status[256];	// windows key code mapping
	uint8_t key_dik_prev[256];
	uint8_t key_converted[256];
	bool    joy_to_key_status[256];

	bool numpad_5_pressed;
	bool key_shift_pressed, key_shift_released;


	uint32_t modkey_status;
	bool lost_focus;
	/*
	 * 0  - 3:
	 * joystick #1, - #4 (b0 = up, b1 = down, b2 = left, b3 = right, b4- = buttons)
	 * 4  - 11:
	 * ANALOG #1 - #4 AXIS LEFT X,Y : VALUE 65536 - 0 (RAW VALUE PLUS 32768)
	 * 12 - 19:
	 * ANALOG #1 - #4 AXIS RIGHT X,Y : VALUE = 65536 - 0 (RAW VALUE PLUS 32768)
	 * 20 - 23:
	 * ANALOG #1 - #4 DIGITAL DIR (b0 = UP, b1 = DOWN, b2 = LEFT, b3 = RIGHT)
	 */
	uint32_t joy_status[32];

	int32_t mouse_status[3];	// x, y, button (b0 = left, b1 = right)
	bool mouse_enabled;
	double mouse_ptrx;
	double mouse_ptry;
    int32_t mouse_button;
	double mouse_oldx;
	double mouse_oldy;
	//Qt::CursorShape mouse_shape;

	QImage background_image;
	QImage button_images[N_MAX_BUTTONS];
	QImage rec_image_buffer;

	// printer

	// screen
	void initialize_screen();
	void release_screen();

	virtual void initialize_screen_buffer(bitmap_t *buffer, int width, int height, int mode);
	void release_screen_buffer(bitmap_t *buffer);
	void rotate_screen_buffer(bitmap_t *source, bitmap_t *dest);
	virtual scrntype_t *get_buffer(bitmap_t *p, int y);

	void stretch_screen_buffer(bitmap_t *source, bitmap_t *dest);
	virtual int add_video_frames();

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
	double m_fps;

	_TCHAR video_file_name[_MAX_PATH];
	int rec_video_fps;

	uint64_t dwAVIFileSize;
	uint64_t lAVIFrames;

	rec_video_thread_param_t rec_video_thread_param;

	bool first_draw_screen;
	bool first_invalidate;
	bool self_invalidate;

	// sound
	void release_sound();
	virtual void init_sound_device_list();
	bool __FASTCALL calcurate_sample_factor(int rate, int samples, const bool force);

	int m_sound_rate, m_sound_samples;
	bool sound_started, now_mute;
	bool sound_first_half;
	QStringList sound_device_list;

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
	uint8_t *sound_buf_ptr;
	Uint8 snd_total_volume;

	// sound capture
	QStringList sound_capture_device_list;
	bool sound_capturing_emu[MAX_CAPTURE_SOUNDS];
	osd_snd_capture_desc_t  sound_capture_desc[MAX_CAPTURE_SOUNDS]; // To EMU:: and VM::
	bool capturing_sound[MAX_SOUND_CAPTURE_DEVICES];
	osd_snd_capture_dev_desc_t  sound_capture_dev_desc[MAX_SOUND_CAPTURE_DEVICES]; // From physical devices
	uint8_t sound_capture_buffer[MAX_SOUND_CAPTURE_DEVICES][32768];
	// video device
	virtual void initialize_video();
	virtual void release_video();

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

	// MIDI : Will implement
	virtual void initialize_midi();
	virtual void release_midi();

	// wrapper
	int max_vm_nodes;
	QList<device_node_t> device_node_list;
	void vm_draw_screen(void);
	Sint16* create_sound(int *extra_frames);

	virtual bool get_use_socket(void);
	virtual bool get_use_auto_key(void);
	virtual bool get_dont_keeep_key_pressed(void);
	virtual bool get_one_board_micro_computer(void);
	virtual bool get_use_screen_rotate(void);
	virtual bool get_use_movie_player(void);
	virtual bool get_use_video_capture(void);
	void vm_key_down(int code, bool flag);
	void vm_key_up(int code);
	void vm_reset(void);

	virtual int get_screen_width(void);
	virtual int get_screen_height(void);
	virtual int get_vm_buttons_code(int num);
	virtual void update_input_mouse();

	// Messaging.
	virtual void __FASTCALL osdcall_message_str(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type, QString message);
	virtual void __FASTCALL osdcall_message_int(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type, int64_t data);
	virtual void __FASTCALL osdcall_mount(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type, QString path);
	virtual void __FASTCALL osdcall_unmount(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type);
	virtual void __FASTCALL osdcall_misc(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type, QString message_str, int64_t data);

public:
	OSD_BASE(std::shared_ptr<USING_FLAGS> p, std::shared_ptr<CSP_Logger> logger);
	~OSD_BASE();

	// common
	VM_TEMPLATE* vm;
	//EMU* emu;
	class Ui_MainWindow *main_window_handle;

	std::recursive_timed_mutex screen_mutex;
	std::recursive_timed_mutex vm_mutex;
	std::recursive_timed_mutex debug_mutex;
	std::recursive_timed_mutex joystick_mutex;
	std::recursive_timed_mutex mouse_mutex;
	std::recursive_timed_mutex log_mutex;
	int host_cpus;
	bool now_auto_key;

	virtual void initialize(int rate, int samples, int* presented_rate, int* presented_samples);
	// sound
	virtual void initialize_sound(int rate, int samples, int* presented_rate, int* presented_samples);

	virtual void release();

	void notify_power_off(); // For USE_NOTIFY_POWER_OFF .

	void power_off();
	void suspend();
	void restore();
	_TCHAR* application_path();
	_TCHAR* bios_path(const _TCHAR* file_name);
	void get_host_time(cur_time_t* time);
	void sleep(uint32_t ms);
	void create_date_file_name(_TCHAR *name, int length, const _TCHAR *extension);
	_TCHAR  *get_app_path(void);
	// common console
	virtual void open_console(int width, int height, const _TCHAR* title);
	virtual void close_console();
	virtual unsigned int get_console_code_page();
	virtual bool is_console_closed();

	void set_console_text_attribute(unsigned short attr);
	void write_console(const _TCHAR* buffer, unsigned int length);
	int read_console_input(_TCHAR* buffer, int length);
	bool is_console_key_pressed(uint32_t ch);
	void update_keyname_table(void);
	// console / telnet
	virtual void open_telnet(const _TCHAR* title);
	virtual void close_telnet();
	virtual void send_telnet(const char* string);

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
	bool is_mouse_enabled();
	//QImage *getPseudoVramClass(void) { return pPseudoVram;}
	void set_mouse_pointer(double x, double y);
	void set_mouse_button(int button);
	void modify_key_buffer(int code, uint8_t val);
	uint8_t* get_key_buffer();
	uint32_t* get_joy_buffer();
	void release_joy_buffer(uint32_t* ptr);
	int32_t get_mouse_button();
	int32_t* get_mouse_buffer();
	void release_mouse_buffer(int32_t* ptr);
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
	int get_vm_screen_width();
	int get_vm_screen_height();

	int get_vm_window_width();
	int get_vm_window_height();
	int get_vm_window_width_aspect();
	int get_vm_window_height_aspect();
	scrntype_t* get_vm_screen_buffer(int y);
	void reset_screen_buffer()
	{
		// It's ugly hack for screen.
		emit sig_resize_vm_screen((QImage*)NULL, -1, -1);
	}
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
	
	// common sound : Moved to SLOT.

	const _TCHAR *get_vm_device_name();
	const _TCHAR *get_sound_device_name(int num);
	QStringList  get_sound_device_list()
	{
		return sound_device_list;
	}

	int get_sound_device_num();

	bool now_record_sound;
	int get_sound_rate();

	// To VM:: and EMU::
	void *get_capture_sound_buffer(int ch);
	bool is_capture_sound_buffer(int ch);
	void *open_capture_sound_emu(int ch, int rate, int channels, int sample_type, int samples, int physical_device_num);
	void close_capture_sound_emu(int ch);

	// From physical device?
	bool open_sound_capture_device(int num, int req_rate, int req_channels);
	bool close_sound_capture_device(int num, bool force);

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
	double vm_frame_rate(void);

	// common socket
	virtual SOCKET get_socket(int ch);
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

	// common MIDI
	virtual void __FASTCALL send_to_midi(uint8_t data, int ch, double timestamp_usec);
	virtual bool __FASTCALL recv_from_midi(uint8_t* data, int ch, double timestamp_usec);
	virtual bool __FASTCALL send_to_midi_timeout(uint8_t data, int ch, uint64_t timeout_ms, double timestamp_usec);
	virtual bool __FASTCALL recv_from_midi_timeout(uint8_t* data, int ch, uint64_t timeout_ms, double timestamp_usec);
	// Note: Belows maybe make Qt SLOTs.
	virtual void __FASTCALL notify_timeout_sending_to_midi(int ch);
	virtual void __FASTCALL notify_timeout_receiving_from_midi(int ch);

	virtual void reset_to_midi(int ch, double timestamp_usec);
	virtual void initialize_midi_device(bool handshake_from_midi, bool handshake_to_midi, int ch);
	virtual void __FASTCALL ready_receive_from_midi(int ch, double timestamp_usec);
	virtual void __FASTCALL ready_send_to_midi(int ch, double timestamp_usec);

	virtual void __FASTCALL request_stop_to_receive_from_midi(int ch, double timestamp_usec);
	virtual void __FASTCALL request_stop_to_send_to_midi(int ch, double timestamp_usec);

	// win32 dependent
	void update_screen();
	void set_parent_thread(EmuThreadClass *parent);
	EmuThreadClass *get_parent_handler();

	_TCHAR *console_input_string(void);
	void clear_console_input_string(void);

	void lock_vm(void);
	void unlock_vm(void);
	void force_unlock_vm(void);
	bool is_vm_locked(void);
	virtual const _TCHAR *get_lib_common_vm_version();
	const _TCHAR *get_lib_common_vm_git_version();
	const _TCHAR *get_lib_osd_version();

	// Wrapper
	virtual void set_draw_thread(DrawThreadClass *handler);
	virtual QString get_vm_config_name(void);
	virtual void reset_vm_node(void);
	// Sync devices status beyond any threads by OSD.(i.e. joystick).
	virtual void sync_some_devices(void);
	
	void set_device_name(int id, char *name);

	void set_vm_node(int id, const _TCHAR *name);
	const _TCHAR *get_vm_node_name(int id);
	int get_vm_node_size(void);

	int get_key_name_table_size(void);
	uint32_t get_scancode_by_vk(uint32_t vk);
	uint32_t get_vk_by_scancode(uint32_t scancode);
	const _TCHAR *get_key_name_by_scancode(uint32_t scancode);
	const _TCHAR *get_key_name_by_vk(uint32_t vk);

	// Get #define S to value.You may use inside of VM/ .
	virtual void set_features(void) {}
	void add_feature(const _TCHAR *key, double value);
	void add_feature(const _TCHAR *key, float value);
	void add_feature(const _TCHAR *key, int value = 1);
	void add_feature(const _TCHAR *key, int64_t value);
	void add_feature(const _TCHAR *key, int16_t value);
	void add_feature(const _TCHAR *key, int8_t value);
	void add_feature(const _TCHAR *key, uint64_t value);
	void add_feature(const _TCHAR *key, uint32_t value);
	void add_feature(const _TCHAR *key, uint16_t value);
	void add_feature(const _TCHAR *key, uint8_t value);
	bool check_feature(const _TCHAR *key);
	double get_feature_double_value(const _TCHAR *key);
	int get_feature_int_value(const _TCHAR *key);
	int64_t get_feature_int64_value(const _TCHAR *key);
	int32_t get_feature_int32_value(const _TCHAR *key);
	int16_t get_feature_int16_value(const _TCHAR *key);
	int8_t get_feature_int8_value(const _TCHAR *key);

	uint64_t get_feature_uint64_value(const _TCHAR *key);
	uint32_t get_feature_uint32_value(const _TCHAR *key);
	uint16_t get_feature_uint16_value(const _TCHAR *key);
	uint8_t get_feature_uint8_value(const _TCHAR *key);

	void debug_log(int level, const char *fmt, ...);
	void debug_log(int level, int domain_num, const char *fmt, ...);
	void debug_log(int level, int domain_num, char *strbuf);
	virtual double get_vm_current_usec() { return 0.0; }
	virtual uint64_t get_vm_current_clock_uint64() { return 0;}

	std::shared_ptr<USING_FLAGS> get_config_flags(void) { return using_flags; }
	// Special
	std::shared_ptr<CSP_Logger> get_logger(void) { return p_logger; }
	virtual bool set_glview(GLDrawClass *glv) { /* Dummy */ return false;}
	QOpenGLContext *get_gl_context();
	virtual GLDrawClass *get_gl_view() { return NULL; }

	// common debugger
	void start_waiting_in_debugger();
	void finish_waiting_in_debugger();
	void process_waiting_in_debugger();

	// Messaging wrapper from EMU:: to OSD::
	void __FASTCALL string_message_from_emu(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t  message_type, _TCHAR* message);
	void __FASTCALL int_message_from_emu(EMU_MEDIA_TYPE::type_t media_type, int drive, EMU_MESSAGE_TYPE::type_t message_type, int64_t data);

public slots:
	// common sound
	void update_sound(int* extra_frames);
	void mute_sound();
	void unmute_sound();
	void stop_sound();
	void start_record_sound();
	void stop_record_sound();
	void restart_record_sound();
	
	void enable_mouse();
	void disable_mouse();
	void toggle_mouse();
	void do_update_joy_status(int num, uint32_t data);

	void upload_bitmap(QImage *p);
	void set_buttons();

	int no_draw_screen();

	void do_write_inputdata(QString s);
	void do_set_input_string(QString s);

	void close_debugger_console();
	void do_close_debugger_thread();

	void do_assign_js_setting(int jsnum, int axis_idx, int assigned_value);
	void do_start_record_video();
	virtual void do_decode_movie(int frames);
	void do_video_movie_end(bool flag);
	void do_video_decoding_error(int num);
	virtual void do_run_movie_audio_callback(uint8_t *data, long len);
	virtual int draw_screen();

	void do_draw(bool flag);

	void set_dbg_completion_list(std::list<std::string> *p);
	void clear_dbg_completion_list(void);
	void set_hdd_image_name(int drv, _TCHAR *filename);

	void do_set_host_sound_output_device(QString device_name);
	void do_update_master_volume(int level);

signals:
	int sig_update_screen(void *, bool);
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

	int sig_movie_open(QString);
	int sig_movie_play();
	int sig_movie_stop();
	int sig_movie_pause(bool);
	int sig_movie_seek_frame(bool, int);
	int sig_movie_mute(bool, bool);
	int sig_movie_eject();
	int sig_movie_quit();

	int sig_set_sound_device(QString);
	int sig_set_sound_volume(double);
	int sig_set_sound_volume(int);
	int sig_sound_mute();
	int sig_sound_unmute();
	int sig_sound_start();
	int sig_sound_stop();
	

	int sig_update_sound_output_list();
	int sig_clear_sound_output_list();
	int sig_append_sound_output_list(QString);

	int sig_update_device_node_name(int id, const _TCHAR *name);
	int sig_enable_mouse(void);
	int sig_disable_mouse(void);

	int sig_reset_joystick();
	
	int sig_close_console(void);
	
	int sig_set_attribute_debugger(QString, bool);
	int sig_move_mouse_to_center(void);
	int sig_clear_dbg_completion_list(void);
	int sig_add_dbg_completion_list(_TCHAR *);
	int sig_apply_dbg_completion_list(void);

	int sig_clear_keyname_table(void);
	int sig_add_keyname_table(uint32_t, QString);

	int sig_change_virtual_media(int, int, QString);

	int sig_notify_power_off(void); // To GUI 20230120 K.O

	// To GUI 20230125 K.O
	int sig_ui_floppy_insert_history(int, QString, quint64);
	int sig_ui_floppy_close(int);
	int sig_ui_floppy_write_protect(int, quint64);

	int sig_ui_quick_disk_insert_history(int, QString);
	int sig_ui_quick_disk_close(int);
	int sig_ui_quick_disk_write_protect(int, quint64);

	int sig_ui_hard_disk_insert_history(int, QString);
	int sig_ui_hard_disk_close(int);

	int sig_ui_cartridge_insert_history(int, QString);
	int sig_ui_cartridge_eject(int);

	int sig_ui_tape_play_insert_history(int, QString);
	int sig_ui_tape_record_insert_history(int, QString);
	int sig_ui_tape_eject(int);
	int sig_ui_tape_position(int, int);
	int sig_ui_tape_message(int, QString);
	int sig_ui_tape_write_protect(int, quint64);

	int sig_ui_tape_push_play(int);
	int sig_ui_tape_push_stop(int);
	int sig_ui_tape_push_fast_forward(int);
	int sig_ui_tape_push_fast_rewind(int);
	int sig_ui_tape_push_apss_forward(int);
	int sig_ui_tape_push_apss_rewind(int);
	int sig_ui_tape_push_pause(int, bool);

	int sig_ui_compact_disc_insert_history(int, QString);
	int sig_ui_compact_disc_eject(int);
	int sig_ui_compact_disc_pause(int);

	int sig_ui_laser_disc_insert_history(int, QString);
	int sig_ui_laser_disc_eject(int);
	int sig_ui_laser_disc_pause(int);

	int sig_ui_binary_loading_insert_history(int, QString);
	int sig_ui_binary_saving_insert_history(int, QString);
	int sig_ui_binary_closed(int);

	int sig_ui_bubble_insert_history(int, QString, quint64);
	int sig_ui_bubble_closed(int);
	int sig_ui_bubble_write_protect(int, quint64);

	// To Logger.
	int sig_debug_log(int, int, QString);
	int sig_logger_reset();
	int sig_logger_set_device_name(int, QString);
	int sig_logger_set_cpu_name(int, QString);
};

QT_END_NAMESPACE

#endif
