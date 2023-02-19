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
#if QT_VERSION >= 0x051400
	#include <QRecursiveMutex>
#else
	#include <QMutex>
#endif

#include <QMutexLocker>

#include "fifo.h"
#include "common.h"
#include "commonclasses.h"
#include "fileio.h"
#include "emu_template.h"

//#include "menuclasses.h"

#include "mainwidget_base.h"
#include "commonclasses.h"
#include "config.h"
#include <memory>

#ifndef MAX_HISTORY
#define MAX_HISTORY 8
#endif
#define MAX_COMMAND_LEN	64

class QWaitCondition;
class QOpenGLContext;

class EMU_TEMPLATE;
class OSD_BASE;
class USING_FLAGS;

class Ui_MainWindowBase;

class VirtualFilesList;
class VirtualBanksList;
class Menu_MetaClass;

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
	EMU_TEMPLATE *p_emu;
	OSD_BASE *p_osd;

	bool poweroff_notified;

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

	std::shared_ptr<USING_FLAGS> using_flags;
	config_t *p_config;

	QWaitCondition *drawCond;
#if QT_VERSION >= 0x051400
	QRecursiveMutex keyMutex;
	QRecursiveMutex mouseMutex;
#else
	QMutex keyMutex;
	QMutex mouseMutex;
#endif
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

#if QT_VERSION >= 0x051400
	QRecursiveMutex uiMutex;
#else
	QMutex uiMutex;
#endif
	char dbg_prev_command[MAX_COMMAND_LEN];

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

	// Standard 8 files.
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

	virtual const _TCHAR *get_device_name(void);
	virtual void resetEmu() { }
	virtual void specialResetEmu(int num) { }
	virtual void loadState() { }
	virtual void saveState() { }

	void enqueue_key_up(key_queue_t s) {
		QMutexLocker n(&keyMutex);
		key_fifo->write(KEY_QUEUE_UP);
		key_fifo->write(s.code);
		key_fifo->write(s.mod);
		key_fifo->write(s.repeat? 1 : 0);
	};
	void enqueue_key_down(key_queue_t s) {
		QMutexLocker n(&keyMutex);
		key_fifo->write(KEY_QUEUE_DOWN);
		key_fifo->write(s.code);
		key_fifo->write(s.mod);
		key_fifo->write(s.repeat? 1 : 0);
	};
	void dequeue_key(key_queue_t *s) {
		QMutexLocker n(&keyMutex);
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
	};
	bool is_empty_key() {
		QMutexLocker n(&keyMutex);
		bool f = key_fifo->empty();
		return f;
	};
	void clear_key_queue() {
		QMutexLocker n(&keyMutex);
		key_fifo->clear();
	};
	// Thread HOOK.
	void sub_close_compact_disc_internal(int drv);
	void sub_close_floppy_disk_internal(int drv);
	void sub_close_hard_disk_internal(int drv);
	void sub_close_laser_disc_internal(int drv);
	void sub_close_quick_disk_internal(int drv);
	void sub_close_tape_internal(int drv);
public:
	EmuThreadClassBase(Ui_MainWindowBase *rootWindow, std::shared_ptr<USING_FLAGS> p, QObject *parent = 0);
	~EmuThreadClassBase();
	virtual void run() {};
	void set_tape_play(bool);
	void resize_screen(int sw, int sh, int stw, int sth);
	void sample_access_drv(void);
	bool now_debugging();

	EMU_TEMPLATE *get_emu() { return p_emu; }

	int get_d88_file_cur_bank(int drive);
	int get_d88_file_bank_num(int drive);
	QString get_d88_file_disk_name(int drive, int banknum);
	bool is_floppy_disk_protected(int drive);
	void set_floppy_disk_protected(int drive, bool flag);
	QString get_d88_file_path(int drive);

public slots:
	void doExit(void);

	void do_reset(void);
	void do_notify_power_off(void);
	void do_special_reset(void);
	void do_load_state();
	void do_save_state();
	void do_update_config();
	void do_start_record_sound();
	void do_stop_record_sound();
	void do_start_record_video();
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
	void do_set_emu_thread_to_fixed_cpu_from_action();
	// From emu_thread_slots.cpp .
	void do_set_display_size(int w, int h, int ww, int wh);
	void moved_mouse(double x, double y, double globalx, double globaly);

	void do_write_protect_floppy_disk(int drv, bool flag);
	void do_close_floppy_disk();
	void do_open_floppy_disk(int drive, QString fname, int slot);
	void do_close_floppy_disk_ui(int drive);

	void do_close_hard_disk();
	void do_open_hard_disk(int, QString);

	void do_play_tape(int drv, QString name);
	void do_rec_tape(int drv, QString name);
	void do_close_tape();

	void do_cmt_push_play();
	void do_cmt_push_stop();
	void do_cmt_push_fast_forward();
	void do_cmt_push_fast_rewind();
	void do_cmt_push_apss_forward();
	void do_cmt_push_apss_rewind();
	void do_cmt_direct_load_from_mzt(bool stat);
	void do_cmt_wave_shaper(bool stat);

	void do_write_protect_quick_disk(int drv, bool flag);
	void do_close_quick_disk();
	void do_close_quick_disk_ui(int drv);
	void do_open_quick_disk(int drv, QString path);


	void do_open_cartridge(int drv, QString path);
	void do_close_cartridge();
	void do_close_cartridge_ui(int drv);

	void do_open_laser_disc(int drv, QString path);
	void do_close_laser_disc();
	void do_close_laser_disc_ui(int drive);

	void do_eject_compact_disc_ui(int drv);
	void do_eject_compact_disc();
	void do_open_compact_disc(int drv, QString path);

	void do_load_binary(int drv, QString path);
	void do_save_binary(int drv, QString path);

	void do_write_protect_bubble_casette(int drv, bool flag);
	void do_close_bubble_casette();
	void do_close_bubble_casette_ui(int drive);
	void do_open_bubble_casette(int drv, QString path, int bank);

	void do_select_floppy_disk_d88(int drive, int slot);

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
	int sig_mouse_enable(bool);
	int sig_update_recent_hard_disk(int);

	int sig_change_osd(int, int, QString);
	int sig_change_access_lamp(int, int, QString);
	int sig_change_virtual_media(int, int, QString);

	int sig_ui_update_config();

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

	// ToDo: Make deprecated.
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

	int sig_set_b77_num(int, int);

	// From emu_thread_slots.cpp .
	int sig_set_draw_fps(double);
	int sig_draw_one_turn(bool);

// Signal from (EMU:: -> OSD:: ->) EMU_THREAD -> GUI(menu_foo[bar])
	int sig_ui_update_quick_disk_list(int, QStringList);
	int sig_ui_close_quick_disk(int);

	int sig_ui_update_compact_disc_list(int, QStringList);
	int sig_ui_close_compact_disc(int);

	int sig_ui_update_cart_list(int, QStringList);
	int sig_ui_close_cart(int);

	int sig_ui_update_binary_list(int, QStringList);
	int sig_ui_close_binary(int);

	int sig_ui_update_bubble_casette_list(int, QStringList);
	int sig_ui_close_bubble_casette(int);
	int sig_ui_clear_b77(int);
	int sig_ui_update_b77(int, int, QString);
	int sig_ui_select_b77(int, int);

};

QT_END_NAMESPACE

#endif
