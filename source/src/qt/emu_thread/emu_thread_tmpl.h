/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	History : 2015.11.10 Split from qt_main.h
	[ win32 main ] -> [ Qt main ] -> [Emu Thread]
*/
#pragma once

#include <QThread>
#include <QQueue>
#include <QString>
#include <QStringList>
#include <QElapsedTimer>
#include <QMap>

#if QT_VERSION >= 0x051400
	#include <QRecursiveMutex>
#else
	#include <QMutex>
#endif

#include <QMutexLocker>

#include "fifo.h"
#include "common.h"
#include "fileio.h"
#include "emu_template.h"

//#include "menuclasses.h"

#include "mainwidget_base.h"
#include "config.h"
#include <memory>
#include <atomic>
#include <utility>

#ifndef MAX_HISTORY
#define MAX_HISTORY 8
#endif
#define MAX_COMMAND_LEN	64

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

typedef enum emu_cpu_affinity_cmd_t{
	CPU_SET_ALL = 0,
	CPU_SET_BIT,
	CPU_CLEAR_BIT,
	CPU_SET_FIXED
} emu_cpu_affinity_cmd_t;

class QTimer;

class DLL_PREFIX EmuThreadClassBase : public QThread {
	Q_OBJECT
protected:
	EMU_TEMPLATE *p_emu;
	OSD_BASE *p_osd;

	Ui_MainWindowBase *MainWindow;

	std::shared_ptr<USING_FLAGS> using_flags;
	config_t *p_config;

    FIFO *key_fifo;
	
#if QT_VERSION >= 0x051400
	QRecursiveMutex m_keyMutex;
//	QRecursiveMutex m_mouseMutex;
	QRecursiveMutex m_uiMutex;
#else
	QMutex m_keyMutex;
//	QMutex m_mouseMutex;
	QMutex m_uiMutex;
#endif
	
	QOpenGLContext *m_glContext;
	std::atomic<bool> is_shared_glcontext;

	QMap<QString, QString> m_virtualMediaList;

	std::atomic<bool> m_poweroff_notified;

	bool m_now_skip;
	
	std::atomic<bool> m_tape_play_flag;
	std::atomic<bool> m_tape_rec_flag;
	std::atomic<int>  m_tape_pos;
	std::atomic<int> m_mouse_x;
	std::atomic<int> m_mouse_y;
	std::list<std::pair<enum emu_cpu_affinity_cmd_t, unsigned int>> m_queue_cpu_affinities;
	
	bool m_prev_record_req;
	
	double m_nr_fps;
	std::atomic<uint32_t> m_led_data_old;
	bool m_req_draw;
	bool m_vert_line_bak;
	bool m_horiz_line_bak;
	bool m_gl_crt_filter_bak;
	int  m_opengl_filter_num_bak;


	std::atomic<uint32_t> m_key_mod;
	Qt::HANDLE m_thread_id;
	QTimer*    m_call_timer;

	QElapsedTimer m_tick_timer;

	std::atomic<bool> m_block_task;
	std::atomic<bool> m_run_thread;
	std::atomic<bool> m_reset_req;
	std::atomic<bool> m_special_reset_req;
	std::atomic<bool> m_load_state_req;
	std::atomic<bool> m_save_state_req;
	std::atomic<bool> m_update_config_req;
	std::atomic<bool> m_start_record_sound_req;
	std::atomic<bool> m_stop_record_sound_req;
	std::atomic<bool> m_start_record_movie_req;
	QString m_save_state_file_name;
	QString m_load_state_file_name;

//	char dbg_prev_command[MAX_COMMAND_LEN];

//	bool draw_timing;
	std::atomic<bool> m_update_volume_req[32];
	std::atomic<int>  m_volume_balance[32];
	std::atomic<int>  m_volume_avg[32];
	std::atomic<int>  m_record_fps;
	std::atomic<int>  m_special_reset_num;
	std::atomic<bool> m_state_power_off;

	bool m_half_count;
	bool m_driven_by_half_of_frame;
	
	bool m_full_speed;
	int64_t m_fps_accum;
	
	int64_t m_current_time;
	int64_t m_next_time;
	int64_t m_update_fps_time;
	bool m_prev_skip;
	int m_total_frames;
	int m_draw_frames;
	int m_skip_frames;
	
	QString qd_text[4];
	QString fd_text[8];
	QString fd_lamp[8];
	QString hdd_text[8];
	QString hdd_lamp[8];
	QString cmt_text[4];
	QString cdrom_text[4];
	QString laserdisc_text[4];
	QString bubble_text[16];

	QString m_clipBoardText;

	// Standard 8 files.
	void calc_volume_from_balance(int num, int balance);
	void calc_volume_from_level(int num, int level);
	int parse_command_queue(QMap<QString, QString> __list);

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

	qint64 get_interval(void);

	virtual const _TCHAR *get_device_name(void);
	void resetEmu();
	void specialResetEmu(int num);
	void loadState();
	void saveState();
	virtual void reset_emulation_values();

	void __FASTCALL enqueue_key_up(key_queue_t s);
	void __FASTCALL enqueue_key_down(key_queue_t s);
	void __FASTCALL dequeue_key(key_queue_t *s);
	bool is_empty_key();
	void clear_key_queue();
	// Thread HOOK.
	void sub_close_bubble_casette_internal(int drv);
	void sub_close_cartridge_internal(int drv);
	void sub_close_compact_disc_internal(int drv);
	void sub_close_floppy_disk_internal(int drv);
	void sub_close_hard_disk_internal(int drv);
	void sub_close_laser_disc_internal(int drv);
	void sub_close_quick_disk_internal(int drv);
	void sub_close_tape_internal(int drv);

	int parse_drive(QString key);
	void parse_file(QString val, QString& filename);
	void parse_file_slot(QString val, QString& filename, bool& protect_changed, bool& is_protected, int& slot );

	virtual void initialize_variables();
	virtual bool initialize_messages();
	virtual int process_command_queue();
	virtual bool check_power_off();
	bool set_led();
	virtual int process_key_input();
	bool __FASTCALL check_scanline_params(bool force);
	
	void __FASTCALL vdebug_log(int level, int domain_num, const char *fmt, va_list __args);
	
	void __FASTCALL debug_log(int level, int domain_num, const char *fmt, ...);
	void __FASTCALL debug_log(int level, const char *fmt, ...);
	void __FASTCALL debug_log(const char *fmt, ...);

	void __FASTCALL info_log(int level, const char *fmt, ...);
	void __FASTCALL info_log(const char *fmt, ...);
	
	void __FASTCALL warn_log(int level, const char *fmt, ...);
	void __FASTCALL warn_log(const char *fmt, ...);
	
	void __FASTCALL debug2_log(int level, const char *fmt, ...);
	void __FASTCALL debug2_log(const char *fmt, ...);
	
	inline int64_t get_current_tick_usec() const
	{
		__LIKELY_IF(m_tick_timer.isValid()) {
			return m_tick_timer.nsecsElapsed() / 1000;
		}
		return 0;
	}

public:
	EmuThreadClassBase(Ui_MainWindowBase *rootWindow, std::shared_ptr<USING_FLAGS> p, QObject *parent = 0);
	~EmuThreadClassBase();
	void set_tape_play(bool);
	void resize_screen(int sw, int sh, int stw, int sth);
	void sample_access_drv(void);
	bool now_debugging();

	EMU_TEMPLATE *get_emu() { return p_emu; }
	void addVirtualMediaList(const QString key, const QString value)
	{
		m_virtualMediaList.insert(key, value);
	}
	ssize_t setVirtualMediaList(const QMap<QString, QString>value)
	{
		m_virtualMediaList.clear();
		for(auto _i = value.constBegin(); _i != value.constEnd(); ++_i) {
			m_virtualMediaList.insert(_i.key(), _i.value());
		}
		return m_virtualMediaList.count();
	}
	int get_d88_file_cur_bank(int drive);
	int get_d88_file_bank_num(int drive);
	QString get_d88_file_disk_name(int drive, int banknum);
	bool is_floppy_disk_protected(int drive);
	void set_floppy_disk_protected(int drive, bool flag);
	QString get_d88_file_path(int drive);

	int get_b77_file_cur_bank(int drive);
	int get_b77_file_bank_num(int drive);
	QString get_b77_file_media_name(int drive, int banknum);
	bool is_bubble_casette_protected(int drive);
	void set_bubble_casette_protected(int drive, bool flag);
	QString get_b77_file_path(int drive);
public slots:
	void doExit(void);
	void do_start(QThread::Priority prio);
	void do_set_priority(QThread::Priority prio);
	
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

	void do_press_button_mouse(Qt::MouseButton);
	void do_release_button_mouse(Qt::MouseButton);
	void do_key_down(uint32_t vk, uint32_t mod, bool repeat);
	void do_key_up(uint32_t vk, uint32_t mod);
	void do_print_framerate(int frames);
	
	void do_set_emu_thread_to_fixed_cpu(int cpunum); // Positive: Assign to cpunum as single thread, Nagative: Assign to all CPUs as multi thread (EMU).
	void do_append_cpu_to_emu_thread(unsigned int cpunum); // Append CPU #cpunum
	void do_remove_cpu_to_emu_thread(unsigned int cpunum); // Remove CPU #cpunum , excepts assigned only one CPU.
	void do_apply_cpu_affinities_to_emu_thread();
	
	void do_block();
	void do_unblock();

	void do_set_emu_thread_to_fixed_cpu_from_action();
	// From emu_thread_slots.cpp .
	void do_set_display_size(int w, int h, int ww, int wh);
	void do_move_mouse(double x, double y, double globalx, double globaly);

	void do_write_protect_floppy_disk(int drv, bool flag);
	void do_close_floppy_disk();
	void do_open_floppy_disk(int drive, QString fname, int slot);
	void do_close_floppy_disk_ui(int drive);

	void do_close_hard_disk();
	void do_close_hard_disk_ui(int drv);
	void do_open_hard_disk(int drv, QString fname);

	void do_play_tape(int drv, QString name);
	void do_rec_tape(int drv, QString name);
	void do_close_tape();
	void do_close_tape_ui(int drv);

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
	void do_select_bubble_casette_b77(int drive, int slot);

	void do_start_auto_key(QString text);
	void do_stop_auto_key(void);
	void do_set_roma_kana(bool flag);
	void do_close_debugger(void);


signals:
	int sig_emu_launched(void);
	int sig_timer_start(int);
	int sig_timer_stop();
	int sig_call_initialize();
	
	//int sig_emu_finished(void);
	
	int message_changed(QString);
	int window_title_changed(QString);
	
	int sig_draw_thread(bool);
	int sig_quit_draw_thread(void);
	int sig_screen_aspect(int);
	int sig_screen_size(int, int);

	int sig_draw_finished(void);
	int sig_mouse_enable(bool);
	int sig_update_recent_hard_disk(int);
	int sig_sound_stop();
	
	int sig_stop_sound_timer();
	int sig_restart_sound_timer();
	
	int sig_change_osd(int, int, QString);
	int sig_change_access_lamp(int, int, QString);
	int sig_change_virtual_media(int, int, QString);

	int sig_ui_update_config();

	int sig_update_recent_bubble(int);
	int sig_change_osd_bubble(int, QString);
	int sig_set_grid_vertical(int, bool);
	int sig_set_grid_horizonal(int, bool);
	int sig_send_data_led(quint32);
	int sig_force_redraw_leds();
	
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
