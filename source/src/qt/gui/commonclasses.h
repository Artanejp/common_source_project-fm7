/*
 * Qt -> GUI -> CommonClasses
 * commonclasses.h
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Licence : GPLv2
 * History : Jan 13 2015 : Split from x1turboz / menuclasses.h
 */

#ifndef _CSP_QT_GUI_COMMONCLASSES_H
#define _CSP_QT_GUI_COMMONCLASSES_H

#include <QVariant>
#include <QObject>
#include <QAction>
#include <QString>

#include "simd_types.h"
#include "common.h"
#include "config.h"

class EMU;
class USING_FLAGS;
//extern class EMU* emu;

QT_BEGIN_NAMESPACE
typedef class DLL_PREFIX Object_Menu_Control: public QObject {
	Q_OBJECT
public:
	USING_FLAGS *using_flags;
Object_Menu_Control(QObject *parent, USING_FLAGS *p) : QObject(parent){
		bindValue = 0;
		drive = 0;
		s_num = 0;
		double_val = 0.0;
		using_flags = p;
		play = true; // Read
		write_protect = false; // Enable to write
		_str.clear();
	}
	~Object_Menu_Control() {}
protected:
	int bindValue;
	int drive;
	int s_num;
	bool play;
	bool write_protect;
	double double_val;
	QString _str;
 public:
	void setValue1(int v) {bindValue = v;}
	int getValue1(void) {return bindValue;}
	void setDrive(int num) { drive = num;}
	int getDrive(void) { return drive;}
	void setNumber(int num) { s_num = num;}
	int getNumber(void) { return s_num;}
	void setDoubleValue(double n) {double_val = n;}
	double getDoubleValue(void) {return double_val;}
	QString getStringValue(void) {return _str; }
	void setStringValue(QString s) { _str = s; }
	bool isPlay(void) { return play; }
	void setPlay(bool b) { play = b; }
   
	bool isWriteProtect(void) { return write_protect; }
	void setWriteProtect(bool b) {write_protect = b;}

public slots:
	void set_boot_mode(void);
	void set_cpu_type(void);
	void set_cpupower(void);
	void open_debugger(void);
	void insert_fd(void);
	void eject_fd(void);
	void on_d88_slot(void);
	void on_recent_disk(void);
	void write_protect_fd(void);
	void no_write_protect_fd(void);
	void do_set_ignore_crc_error(bool flag);
	void do_set_correct_disk_timing(bool flag);
	void do_set_disk_count_immediate(bool flag);
	// Bubble
	void insert_bubble(void);
	void eject_bubble(void);
	void on_b77_slot(void);
	void on_recent_bubble(void);
	void write_protect_bubble(void);
	void no_write_protect_bubble(void);

	void set_screen_aspect(void);
	void set_screen_size(void);
	void do_set_device_type(void);
	void do_set_keyboard_type(void);
	void do_set_joystick_type(void);
	void do_set_mouse_type(void);
	void do_set_drive_type(void);
	void do_set_sound_device(void);
	void do_set_printer_device(void);
	
	void insert_Qd(void);
	void eject_Qd(void);
	void on_recent_quick_disk(void);
	void write_protect_Qd(void);
	void no_write_protect_Qd(void);
	
	void on_recent_binary_load(void);
	void on_recent_binary_save(void);
	void _open_binary(QString s);
	void insert_binary_load(void);
	void insert_binary_save(void);
	
	void insert_laserdisc(void);
	void eject_laserdisc(void);
	void on_recent_laserdisc();
	
	void on_set_freq(void);
	void on_set_latency(void);

	void insert_cart(void);
	void eject_cart(void);
	void on_recent_cart(void);
	void do_save_as_movie(void);
	void do_stop_saving_movie(void);
	void do_set_monitor_type();
signals:
	int on_boot_mode(int);   
	int on_cpu_type(int);   
	int on_cpu_power(int); 
	int on_open_debugger(int);
	
	int sig_insert_fd(int);
	int sig_eject_fd(int);
	int set_d88_slot(int, int);
	int set_recent_disk(int, int);
	int sig_write_protect_fd(int, bool);
	int sig_device_type(int);
	int sig_keyboard_type(int);
	int sig_joystick_type(int);
	int sig_mouse_type(int);
	int sig_sound_device(int);
	int sig_printer_device(int);
	int sig_drive_type(int);
	int sig_emu_update_config(void);

	int set_recent_quick_disk(int, int);
	int sig_write_protect_Qd(int, bool);
	int sig_eject_Qd(int);
	int sig_insert_Qd(int);
	
	int sig_recent_cdrom(int);
		
	int sig_insert_laserdisc(bool);
	int sig_eject_laserdisc(void);
	int sig_recent_laserdisc(int);
	
	int sig_insert_cart(int);
	int sig_eject_cart(int);
	int set_recent_cart(int, int);

	int sig_open_binary_file(int, QString, bool);
	int sig_open_binary(int, bool);
	int set_recent_binary_load(int, int);
	int set_recent_binary_save(int, int);

	// bubble
	int sig_insert_bubble(int);
	int sig_eject_bubble(int);
	int set_b77_slot(int, int);
	int set_recent_bubble(int, int);
	int sig_write_protect_bubble(int, bool);
	
	int sig_freq(int);
	int sig_latency(int);
	int sig_sounddevice(int);
	int sig_set_dipsw(int, bool);
	int sig_screen_aspect(int);
	int sig_screen_size(int, int);
	int sig_screen_multiply(float);

	int sig_monitor_type(int);
	
signals:
	int sig_stop_record_movie();
	int sig_start_record_movie(int);
} Object_Menu_Control ;



typedef class DLL_PREFIX Action_Control: public QAction {
	Q_OBJECT
  protected:
	//    virtual void addedTo ( QWidget * actionWidget, QWidget * container ){}
	//   virtual void addedTo ( int index, QPopupMenu * menu ){}
	QString bindString;
 public:
	Object_Menu_Control *binds;
	Action_Control (QObject *parent, USING_FLAGS *p) : QAction(parent) {
		binds = new Object_Menu_Control(parent, p);
		bindString.clear();
	}
	~Action_Control() {
		delete binds;
	}
public slots:
  	void do_check_grab_mouse(bool);
	void do_send_string(void);
	void do_set_string(QString);
	void do_set_dev_log_to_console(bool f);
	void do_set_dev_log_to_syslog(bool f);
	void do_select_render_platform(void);
	void do_save_state(void);
	void do_load_state(void);
	void do_set_window_focus_type(bool flag);
	void do_set_emulate_cursor_as(void);
	
signals:
	int quit_emu_thread(void);
	int sig_send_string(QString);
	int sig_set_dev_log_to_console(int, bool);
	int sig_set_dev_log_to_syslog(int, bool);
	int sig_select_render_platform(int);
	int sig_save_state(QString);
	int sig_load_state(QString);
	int sig_set_window_focus_type(bool);
	void sig_set_emulate_cursor_as(int);
} ActionControl;
QT_END_NAMESPACE

#endif


 
