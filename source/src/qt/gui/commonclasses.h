/*
 * Qt -> GUI -> CommonClasses
 * commonclasses.h
 * (C) 2015 K.Ohta <whatisthis.sowhat _at_ gmail.com>
 * Licence : GPLv2
 * History : Jan 13 2015 : Split from x1turboz / menuclasses.h
 */

#ifndef _CSP_QT_GUI_COMMONCLASSES_H
#define _CSP_QT_GUI_COMMONCLASSES_H

#if defined(_USE_QT5)
#include <QVariant>
#include <QAction>
#include <QApplication>
#include <QButtonGroup>
#include <QGraphicsView>
#include <QHeaderView>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QStatusBar>
#include <QWidget>
#include <QIcon>
#include <QString>
#else
#include <QtCore/QVariant>
#include <QtCore/QString>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGraphicsView>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>
#include <QtGui/QIconSet>
#endif

#include "simd_types.h"
#include "common.h"
#include "config.h"
//#include "emu.h"
//#include "qt_main.h"
//#include "qt_gldraw.h"

class EMU;
extern class EMU* emu;

QT_BEGIN_NAMESPACE
typedef class Object_Menu_Control: public QObject {
	Q_OBJECT
public:
//   explicit Object_Menu_Control(QObject *parent = 0);
	Object_Menu_Control(QObject *parent) : QObject(parent){
		bindValue = 0;
		drive = 0;
		s_num = 0;
		double_val = 0.0;
		play = true; // Read
		write_protect = false; // Enable to write
	}
	~Object_Menu_Control() {}
private:
	int bindValue;
	int drive;
	int s_num;
	bool play;
	bool write_protect;
	double double_val; 
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
	int sig_sound_device(int);
	int sig_printer_device(int);
	int sig_drive_type(int);
	int sig_emu_update_config(void);

	int set_recent_quick_disk(int, int);
	int sig_write_protect_Qd(int, bool);
	int sig_eject_Qd(int);
	int sig_insert_Qd(int);
	
	int sig_insert_cdrom(bool);
	int sig_eject_cdrom(void);
	int sig_recent_cdrom(int);
		
	int sig_insert_play_cmt(bool);
	int sig_eject_cmt(void);
	int sig_recent_cmt(int);
	int sig_set_write_protect_cmt(bool);
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
public slots:
	void set_boot_mode(void);
	void set_cpu_type(void);
	void set_cpupower(void);
	void open_debugger(void);
	void do_set_write_protect_cmt(void);
	void do_unset_write_protect_cmt(void);
	void insert_fd(void);
	void eject_fd(void);
	void on_d88_slot(void);
	void on_recent_disk(void);
	void write_protect_fd(void);
	void no_write_protect_fd(void);
	void do_set_ignore_crc_error(bool flag);
	void do_set_correct_disk_timing(bool flag);
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
	
	void insert_cdrom(void);
	void eject_cdrom(void);
	void on_recent_cdrom();

	void start_insert_play_cmt(void);
	void eject_cmt(void);
	void on_recent_cmt(void);
   
	void on_set_freq(void);
	void on_set_latency(void);

	void insert_cart(void);
	void eject_cart(void);
	void on_recent_cart(void);
     
 public:
	void setValue1(int v) {bindValue = v;}
	int getValue1(void) {return bindValue;}
	void setDrive(int num) { drive = num;}
	int getDrive(void) { return drive;}
	void setNumber(int num) { s_num = num;}
	int getNumber(void) { return s_num;}
	void setDoubleValue(double n) {double_val = n;}
	double getDoubleValue(void) {return double_val;}
	
	bool isPlay(void) { return play; }
	void setPlay(bool b) { play = b; }
   
	bool isWriteProtect(void) { return write_protect; }
	void setWriteProtect(bool b) {write_protect = b;}
} Object_Menu_Control ;

typedef class Action_Control: public QAction {
	Q_OBJECT
  protected:
	//    virtual void addedTo ( QWidget * actionWidget, QWidget * container ){}
	//   virtual void addedTo ( int index, QPopupMenu * menu ){}
	QString bindString;
 public:
	Object_Menu_Control *binds;
	Action_Control (QObject *parent) : QAction(parent) {
		binds = new Object_Menu_Control(parent);
		bindString.clear();
	}
	~Action_Control() {
		delete binds;
	}
public slots:
  	void do_check_grab_mouse(bool);
	void do_send_string(void);
	void do_set_string(QString);
signals:
	int quit_emu_thread(void);
	int sig_send_string(QString);
} ActionControl;
QT_END_NAMESPACE

#endif


 
