/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	History : 2015.11.10 Split from qt_main.h
	[ win32 main ] -> [ Qt main ] -> [Emu Thread]
*/
#ifndef _CSP_QT_EMU_THREAD_TMPL_H
#define _CSP_QT_EMU_THREAD_TMPL_H

#include <QThread>
#include <QQueue>
#include <QString>
#include <QStringList>
#include <QElapsedTimer>

#include "fifo.h"
#include "common.h"
#include "commonclasses.h"
#include "fileio.h"
#include "emu.h"

//#include "menuclasses.h"
#include "mainwidget_base.h"
#include "commonclasses.h"
#include "config.h"

#ifndef MAX_HISTORY
#define MAX_HISTORY 8
#endif
#define MAX_COMMAND_LEN	64

class EMU;
class QWaitCondition;
class QOpenGLContext;
class USING_FLAGS;
class Ui_MainWindowBase;
class OSD_BASE;
//class META_MainWindow;

QT_BEGIN_NAMESPACE
enum {
	KEY_QUEUE_DOWN = 0x10000000,
	KEY_QUEUE_UP   = 0x20000000,
};	
typedef struct {
	uint32_t type;
	uint32_t code;
	uint32_t mod;
	bool repeat;
} key_queue_t;

class DLL_PREFIX EmuThreadClassBase : public QThread {
	Q_OBJECT
protected:
	EMU *p_emu;
	OSD_BASE *p_osd;
	
	bool now_skip;
	bool calc_message;
	bool tape_play_flag;
	bool tape_rec_flag;
	int tape_pos;
	bool mouse_flag;
	int mouse_x;
	int mouse_y;
	Qt::HANDLE thread_id;
	int queue_fixed_cpu;

    FIFO *key_fifo;
	QOpenGLContext *glContext;
	bool is_shared_glcontext;
	
	uint32_t key_mod;

	USING_FLAGS *using_flags;
	config_t *p_config;
	
	QWaitCondition *drawCond;
	QMutex *keyMutex;
	
	//class META_MainWindow *MainWindow;
	Ui_MainWindowBase *MainWindow;
	QElapsedTimer tick_timer;
	
	bool bBlockTask;
	bool bRunThread;
	bool bResetReq;
	bool bSpecialResetReq;
	bool bLoadStateReq;
	bool bSaveStateReq;
	bool bUpdateConfigReq;
	bool bStartRecordSoundReq;
	bool bStopRecordSoundReq;
	bool bStartRecordMovieReq;
	QString sStateFile;
	QString lStateFile;

	QMutex uiMutex;
	char dbg_prev_command[MAX_COMMAND_LEN];
	int fd_open_wait_count[8];
	QString fd_reserved_path[8];
	int fd_reserved_bank[8];
	
//	bool draw_timing;
	bool doing_debug_command;
	bool bUpdateVolumeReq[32];
	int volume_balance[32];
	int volume_avg[32];
	int record_fps;
	int  specialResetNum;

	qint64 next_time;
	qint64 update_fps_time;
	bool prev_skip;
	int total_frames;
	int draw_frames;
	int skip_frames;
	QString qd_text[4];
	QString fd_text[8];
	QString fd_lamp[8];
	QString hdd_text[8];
	QString hdd_lamp[8];
	QString cmt_text[4];
	QString cdrom_text[4];
	QString laserdisc_text[4];
	QString bubble_text[16];
	
	QString clipBoardText;
	QStringList vMovieQueue;

	void calc_volume_from_balance(int num, int balance);
	void calc_volume_from_level(int num, int level);
	int parse_command_queue(QStringList _l, int _begin);
	
	void button_pressed_mouse_sub(Qt::MouseButton button);
	void button_released_mouse_sub(Qt::MouseButton button);
	
	void get_qd_string(void);
	void get_fd_string(void);
	void get_hdd_string(void);
	void get_tape_string(void);
	void get_cd_string(void);
	void get_bubble_string(void);

	const _TCHAR *get_emu_message(void);
	double get_emu_frame_rate(void);
	int get_message_count(void);
	void dec_message_count(void);
	bool get_power_state(void);

	virtual const _TCHAR *get_device_name(void);
	virtual void resetEmu() { }
	virtual void specialResetEmu(int num) { }
	virtual void loadState() { }
	virtual void saveState() { }

	void enqueue_key_up(key_queue_t s) {
		keyMutex->lock();
		key_fifo->write(KEY_QUEUE_UP);
		key_fifo->write(s.code);
		key_fifo->write(s.mod);
		key_fifo->write(s.repeat? 1 : 0);
		keyMutex->unlock();
	};
	void enqueue_key_down(key_queue_t s) {
		keyMutex->lock();
		key_fifo->write(KEY_QUEUE_DOWN);
		key_fifo->write(s.code);
		key_fifo->write(s.mod);
		key_fifo->write(s.repeat? 1 : 0);
		keyMutex->unlock();
	};
	void dequeue_key(key_queue_t *s) {
		keyMutex->lock();
		uint32_t _type = (uint32_t)key_fifo->read();
		if(_type == 	KEY_QUEUE_DOWN) {
			s->type = _type;
			s->code = (uint32_t)key_fifo->read();
			s->mod  = (uint32_t)key_fifo->read();
			if(key_fifo->read() != 0) {
				s->repeat = true;
			} else {
				s->repeat = false;
			}
		} else if(_type == KEY_QUEUE_UP) {
			s->type = _type;
			s->code = (uint32_t)key_fifo->read();
			s->mod  = (uint32_t)key_fifo->read();
			volatile uint32_t dummy = key_fifo->read();
			s->repeat = false;
		} else {
			s->type = 0;
			s->code = 0;
			s->mod = 0;
			s->repeat = false;
		}
		keyMutex->unlock();
	};
	bool is_empty_key() {
		keyMutex->lock();
		bool f = key_fifo->empty();
		keyMutex->unlock();
		return f;
	};
	void clear_key_queue() {
		keyMutex->lock();
		key_fifo->clear();
		keyMutex->unlock();
	};

public:
	EmuThreadClassBase(Ui_MainWindowBase *rootWindow, USING_FLAGS *p, QObject *parent = 0);
	~EmuThreadClassBase();
	virtual void run() {};
	void set_tape_play(bool);
	void resize_screen(int sw, int sh, int stw, int sth);
	void sample_access_drv(void);
	bool now_debugging();

	int get_d88_file_cur_bank(int drive);
	int get_d88_file_bank_num(int drive);
	QString get_d88_file_disk_name(int drive, int banknum);
	bool is_floppy_disk_protected(int drive);
	void set_floppy_disk_protected(int drive, bool flag);
	QString get_d88_file_path(int drive);

public slots:
	void doExit(void);
	
	void do_reset();
	void do_special_reset(int num);
	void do_load_state(QString name);
	void do_save_state(QString name);
	void do_update_config();
	void do_start_record_sound();
	void do_stop_record_sound();
	void do_start_record_video(int fps);
	void do_stop_record_video();
	void do_update_volume_level(int num, int level);
	void do_update_volume_balance(int num, int level);

	void button_pressed_mouse(Qt::MouseButton);
	void button_released_mouse(Qt::MouseButton);
	void do_key_down(uint32_t vk, uint32_t mod, bool repeat);
	void do_key_up(uint32_t vk, uint32_t mod);
	void print_framerate(int frames);
	void set_emu_thread_to_fixed_cpu(int cpunum);
	void do_block();
	void do_unblock();
	void do_start_emu_thread();

	// From emu_thread_slots.cpp .
	void do_set_display_size(int w, int h, int ww, int wh);
	void moved_mouse(int x, int y, int globalx, int globaly);

	void do_write_protect_disk(int drv, bool flag);
	void do_close_disk(int);
	void do_open_disk(int, QString, int);
	void do_close_hard_disk(int);
	void do_open_hard_disk(int, QString);
	void do_play_tape(int drv, QString name);
	void do_rec_tape(int drv, QString name);
	void do_close_tape(int drv);
	void do_cmt_push_play(int drv);
	void do_cmt_push_stop(int drv);
	void do_cmt_push_fast_forward(int drv);
	void do_cmt_push_fast_rewind(int drv);
	void do_cmt_push_apss_forward(int drv);
	void do_cmt_push_apss_rewind(int drv);
	void do_write_protect_quickdisk(int drv, bool flag);
	void do_close_quickdisk(int drv);
	void do_open_quickdisk(int drv, QString path);
	void do_close_cart(int drv);
	void do_open_cart(int drv, QString path);
	void do_close_laser_disc(int drv);
	void do_open_laser_disc(int drv, QString path);
	void do_eject_cdrom(int drv);
	void do_open_cdrom(int drv, QString path);
	void do_load_binary(int drv, QString path);
	void do_save_binary(int drv, QString path);
	void do_write_protect_bubble_casette(int drv, bool flag);
	void do_close_bubble_casette(int);
	void do_open_bubble_casette(int, QString, int);
	void do_start_auto_key(QString text);
	void do_stop_auto_key(void);
	void set_romakana(bool flag);
	void do_close_debugger(void);

signals:
	int message_changed(QString);
	int window_title_changed(QString);
	int sig_draw_thread(bool);
	int quit_draw_thread(void);
	int sig_screen_aspect(int);
	int sig_screen_size(int, int);
	int sig_finished(void);
	int call_emu_thread(EMU *);
	int sig_check_grab_mouse(bool);
	int sig_mouse_enable(bool);
	int sig_update_recent_disk(int);
	int sig_update_recent_hard_disk(int);
	
	int sig_change_osd(int, int, QString);
	int sig_change_access_lamp(int, int, QString);
	int sig_change_virtual_media(int, int, QString);
	
	int sig_update_recent_bubble(int);
	int sig_change_osd_bubble(int, QString);
	int sig_set_grid_vertical(int, bool);
	int sig_set_grid_horizonal(int, bool);
	int sig_send_data_led(quint32);
	int sig_resize_screen(int, int);
	int sig_resize_uibar(int, int);
	int sig_resize_osd(int);
	int sig_is_enable_mouse(bool);
	int sig_debugger_input(QString);
	int sig_quit_debugger();
	int sig_romakana_mode(bool);
	int sig_set_access_lamp(int, bool);

	int sig_open_binary_load(int, QString);
	int sig_open_binary_save(int, QString);
	int sig_open_cart(int, QString);
	int sig_open_cmt_load(int, QString);
	int sig_open_cmt_write(int, QString);
	int sig_open_fd(int, QString);
	int sig_open_d88_fd(int, QString, int);
	int sig_open_hdd(int, QString);
	
	int sig_open_quick_disk(int, QString);
	int sig_open_bubble(int, QString);
	int sig_open_b77_bubble(int, QString, int);
	int sig_open_cdrom(int, QString);
	int sig_open_laser_disc(int, QString);
	
	int sig_set_d88_num(int, int);
	int sig_set_b77_num(int, int);

	// From emu_thread_slots.cpp .
	int sig_set_draw_fps(double);
	int sig_draw_one_turn(bool);
	int sig_update_d88_list(int, int);
};

QT_END_NAMESPACE

#endif
