/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	History : 2015.11.10 Split from qt_main.h
	[ win32 main ] -> [ Qt main ] -> [Emu Thread]
*/
#ifndef _CSP_QT_EMU_THREAD_H
#define _CSP_QT_EMU_THREAD_H

#include <QThread>
#include <QElapsedTimer>

#include "common.h"
#include "commonclasses.h"
#include "fileio.h"
#include "emu.h"
#include "vm.h"
#include "menuclasses.h"
#include "mainwidget.h"
#include "commonclasses.h"

class META_MainWindow;
class EMU;
class QString;
class QWaitCondition;

#ifndef MAX_HISTORY
#define MAX_HISTORY 8
#endif

QT_BEGIN_NAMESPACE
#define MAX_COMMAND_LEN	64

class EmuThreadClass : public QThread {
	Q_OBJECT
private:
	bool calc_message;
	bool tape_play_flag;
	bool tape_rec_flag;
	int tape_pos;
	bool mouse_flag;
	char dbg_prev_command[MAX_COMMAND_LEN];

	int get_interval(void);
	
 protected:
	EMU *p_emu;
	QWaitCondition *drawCond;
	class META_MainWindow *MainWindow;
	QElapsedTimer tick_timer;
	
	bool bRunThread;
	bool bResetReq;
	bool bSpecialResetReq;
	bool bLoadStateReq;
	bool bSaveStateReq;
	bool bUpdateConfigReq;
	bool bStartRecordSoundReq;
	bool bStopRecordSoundReq;
	bool draw_timing;
	bool doing_debug_command;
	
	qint64 next_time;
	qint64 update_fps_time;
	bool prev_skip;
	int total_frames;
	int draw_frames;
	int skip_frames;
#if defined(USE_QD1)
	QString qd_text[8];
#endif
#if defined(USE_FD1)
	QString fd_text[MAX_FD];
#endif
#if defined(USE_TAPE)
	QString cmt_text;
#endif
#ifdef USE_AUTO_KEY
	QString clipBoardText;
#endif
	void sample_access_drv(void);
public:
	EmuThreadClass(META_MainWindow *rootWindow, EMU *pp_emu, QObject *parent = 0);
	~EmuThreadClass();
	void SetEmu(EMU *p) {
		p_emu = p;
	}
	void set_tape_play(bool);
	void run() { doWork("");}
	EmuThreadClass *currentHandler();
	void resize_screen(int sw, int sh, int stw, int sth);
	bool now_debugging();
public slots:
	void doWork(const QString &param);
	void doExit(void);
	void print_framerate(int frames);
	void doReset();
	void doSpecialReset();
	void doLoadState();
	void doSaveState();
	void doUpdateConfig();
	void doStartRecordSound();
	void doStopRecordSound();
	void doSetDisplaySize(int w, int h, int ww, int wh);
	
	void moved_mouse(int, int);
	void button_pressed_mouse(Qt::MouseButton);
	void button_released_mouse(Qt::MouseButton);
#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
	void do_write_protect_disk(int drv, bool flag);
	void do_close_disk(int);
	void do_open_disk(int, QString, int);
#endif
#ifdef USE_TAPE
	void do_play_tape(QString name);
	void do_rec_tape(QString name);
	void do_close_tape(void);
# ifdef USE_TAPE_BUTTON
	void do_cmt_push_play(void);
	void do_cmt_push_stop(void);
	void do_cmt_push_fast_forward(void);
	void do_cmt_push_fast_rewind(void);
	void do_cmt_push_apss_forward(void);
	void do_cmt_push_apss_rewind(void);
# endif
#endif // USE_TAPE
#ifdef USE_QD1	
	void do_write_protect_quickdisk(int drv, bool flag);
	void do_close_quickdisk(int drv);
	void do_open_quickdisk(int drv, QString path);
#endif
#ifdef USE_CART1
	void do_close_cart(int drv);
	void do_open_cart(int drv, QString path);
#endif
#ifdef USE_LASER_DISK
	void do_close_laser_disk(void);
	void do_open_laser_disk(QString path);
#endif
#ifdef USE_BINARY_FILE1
	void do_load_binary(int drv, QString path);
	void do_save_binary(int drv, QString path);
#endif
	void do_start_auto_key(QString text);
	void do_stop_auto_key(void);
	void do_draw_timing(bool);
	void do_call_debugger_command(QString s);
	void do_close_debugger(void);
	
signals:
	int message_changed(QString);
	int sig_draw_thread(bool);
	int quit_draw_thread(void);
	int sig_screen_aspect(int);
	int sig_screen_size(int, int);
	int sig_finished(void);
	int call_emu_thread(EMU *);
	int sig_check_grab_mouse(bool);
	int sig_mouse_enable(bool);
#if defined(USE_FD1) || defined(USE_FD2) || defined(USE_FD3) || defined(USE_FD4) || \
    defined(USE_FD5) || defined(USE_FD6) || defined(USE_FD7) || defined(USE_FD8)
	int sig_update_recent_disk(int);
	int sig_change_osd_fd(int, QString);
#endif
#if defined(USE_QD1) || defined(USE_QD2) || defined(USE_QD3) || defined(USE_QD4)
	int sig_change_osd_qd(int, QString);
#endif
#if defined(USE_TAPE)
	int sig_change_osd_cmt(QString);
#endif
#if defined(USE_DIG_RESOLUTION)
	int sig_set_grid_vertical(int, bool);
	int sig_set_grid_horizonal(int, bool);
#endif	
#ifdef SUPPORT_DUMMY_DEVICE_LED
	int sig_send_data_led(quint32);
#endif
	int sig_resize_screen(int, int);
	int sig_resize_uibar(int, int);
	int sig_auto_key_string(QByteArray);
	int sig_debugger_input(QString);
	int sig_quit_debugger();
};

QT_END_NAMESPACE

#endif
