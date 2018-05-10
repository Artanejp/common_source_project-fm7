/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

#ifndef _QT_OSD_H_
#define _QT_OSD_H_

#include "osd_base.h"
#include "gui/qt_input.h" // Key code table (VK_foo).

class GLDrawClass;
class EmuThreadClass;
class DrawThreadClass;
class Ui_MainWindow;
class EMU;
class VM;
class FIFO;
class USING_FLAGS;
class CSP_KeyTables;
class MOVIE_LOADER;
class SOUND_LOADER;
class QTcpSocket2;
class QUdpSocket2;

QT_BEGIN_NAMESPACE	
class CSP_Logger;
class OSD : public OSD_BASE
{
	Q_OBJECT
private:
	void set_features_machine(void);
	void set_features_cpu(void);
	void set_features_vm(void);
	void set_features_misc(void);
	void set_features_debug(void);
protected:
	void vm_draw_screen(void);
	Sint16* create_sound(int *extra_frames);
	bool get_use_socket(void);
	bool get_support_variable_timing(void);
	bool get_notify_key_down(void);
	bool get_notify_key_down_lr_shift(void);
	bool get_notify_key_down_lr_control(void);
	bool get_notify_key_down_lr_menu(void);
	bool get_use_shift_numpad_key(void);
	bool get_use_auto_key(void);
	bool get_dont_keeep_key_pressed(void);
	bool get_one_board_micro_computer(void);
	bool get_use_screen_rotate(void);
	bool get_use_movie_player(void);
	bool get_use_video_capture(void);
	void vm_key_down(int code, bool flag);
	void vm_key_up(int code);
	void vm_reset(void);
	void update_buttons(bool press_flag, bool release_flag);
	int get_screen_width(void);
	int get_screen_height(void);
	int get_vm_buttons_code(int num);

	void set_features(void);
	void set_device_name(int id, char *name);
	MOVIE_LOADER *movie_loader;

	QTcpSocket2 *tcp_socket[SOCKET_MAX];
	QUdpSocket2 *udp_socket[SOCKET_MAX];
	

#ifdef USE_SOUND_FILES
	SOUND_LOADER *tail_sound_file;
	SOUND_LOADER *sound_file_obj[USE_SOUND_FILES];
	
	void init_sound_files();
	void release_sound_files();
#endif
public:
	OSD(USING_FLAGS *p, CSP_Logger *logger);
	~OSD();
	void initialize(int rate, int samples);
	void release();
	void power_off();

	// Wrapper
	// Locker
	void lock_vm(void);
	void unlock_vm(void);
	void force_unlock_vm(void);
	bool is_vm_locked(void);

	// Screen
	void set_draw_thread(DrawThreadClass *handler);
	void initialize_screen();
	void release_screen();
	int get_window_mode_width(int mode);
	int get_window_mode_height(int mode);
	QString get_vm_config_name(void);
	double vm_frame_rate(void);

	// Movie/Video
	void get_video_buffer();
	void initialize_video();
	void release_video();
	bool open_movie_file(const _TCHAR* file_path);
	void close_movie_file();
	uint32_t get_cur_movie_frame();
	int get_movie_sound_rate();

	// Misc
	void reset_vm_node(void);

	// Socket
	void initialize_socket();
	void release_socket();
	void notify_socket_connected(int ch);
	void notify_socket_disconnected(int ch);
	void update_socket();
	bool initialize_socket_tcp(int ch);
	bool initialize_socket_udp(int ch);
	bool connect_socket(int ch, uint32_t ipaddr, int port);
	void disconnect_socket(int ch);
	bool listen_socket(int ch);
	void send_socket_data_tcp(int ch);
	void send_socket_data_udp(int ch, uint32_t ipaddr, int port);
	void send_socket_data(int ch);
	void recv_socket_data(int ch);
	int get_socket(int ch);

	// Sound
#ifdef USE_SOUND_FILES
	void load_sound_file(int id, const _TCHAR *name, int16_t **data, int *dst_size);
	void free_sound_file(int id, int16_t **data);
#endif
	void debug_log(int level, int domain_num, char *strbuf);
public slots:
	void do_decode_movie(int frames);
	void do_run_movie_audio_callback(uint8_t *data, long len);
	void do_notify_socket_connected(int ch);
	void do_notify_socket_disconnected(int ch);
};
QT_END_NAMESPACE

#endif
