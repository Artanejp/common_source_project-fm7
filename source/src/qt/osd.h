/*
	Skelton for retropc emulator

	Author : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2015.11.30-

	[ Qt dependent ]
*/

#ifndef _QT_OSD_H_
#define _QT_OSD_H_


#include <QObject>
#include <QString>
#include <QImage>

#include <SDL.h>
#include "osd_base.h"

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

QT_BEGIN_NAMESPACE
class OSD : public OSD_BASE
{
	Q_OBJECT
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
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	MOVIE_LOADER *movie_loader;
#endif	
public:
	OSD(USING_FLAGS *p);
	~OSD();
	void initialize(int rate, int samples);
	void release();
	void power_off();

	// Wrapper
	void lock_vm(void);
	void unlock_vm(void);
	void force_unlock_vm(void);
	bool is_vm_locked(void);
	void set_draw_thread(DrawThreadClass *handler);
	void initialize_screen();
	void release_screen();
	int get_window_mode_width(int mode);
	int get_window_mode_height(int mode);
	QString get_vm_config_name(void);
	double vm_frame_rate(void);
	
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	void initialize_video();
	void release_video();
	bool open_movie_file(const _TCHAR* file_path);
	void close_movie_file();
	uint32_t get_cur_movie_frame();
#endif
public slots:
#if defined(USE_MOVIE_PLAYER) || defined(USE_VIDEO_CAPTURE)
	void do_decode_movie(int frames);
	void do_run_movie_audio_callback(uint8_t *data, long len);
#endif
};
QT_END_NAMESPACE

#endif
