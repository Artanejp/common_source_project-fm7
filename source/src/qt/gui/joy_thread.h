/*
	Skelton for retropc emulator
	Author : Takeda.Toshiya
        Port to Qt : K.Ohta <whatisthis.sowhat _at_ gmail.com>
	Date   : 2006.08.18 -
	License : GPLv2
	History : 2015.11.10 Split from qt_main.h
	[ win32 main ] -> [ Qt main ] -> [Joy Stick]
*/
#ifndef _CSP_QT_JOY_THREAD_H
#define _CSP_QT_JOY_THREAD_H

#include <memory>
#include <QThread>
#include <SDL.h>
#include <QStringList>
#include <QString>
#include "common.h"
#include "config.h"

class EmuThreadClassBase;
class OSD_BASE;
class USING_FLAGS;

QT_BEGIN_NAMESPACE
#ifndef JOY_DB_NAME
#define JOY_DB_NAME "joydb.ini"
#endif
class DLL_PREFIX JoyThreadClass : public QThread {
	Q_OBJECT
 private:
	int joy_num[16];
	SDL_Event event;
#if defined(USE_SDL2)   
	SDL_GameController *controller_table[16];
#endif	
	SDL_Joystick *joyhandle[16];
	QString names[16];
	int joy_assign[16];
	bool emulate_dpad[4];
	bool is_controller[16];
	
	std::shared_ptr<EmuThreadClassBase> p_emu_thread;
	OSD_BASE *p_osd;
	std::shared_ptr<USING_FLAGS> using_flags;
	config_t *p_config;
	QStringList joydb;
	QString default_assign;

	int write_joydb();
	int read_joydb();

 protected:
	bool bRunThread;
	std::shared_ptr<CSP_Logger> csp_logger;
	
	void joystick_plugged(int num);
	void joystick_unplugged(int num);
	bool EventSDL(SDL_Event *);
	void x_axis_changed(int, int, int);
	void y_axis_changed(int, int, int);
	void button_down(int, unsigned int);
	void button_up(int, unsigned int);
	void controller_button_down(int, unsigned int);
	void controller_button_up(int, unsigned int);
	int get_joy_num(int id);
# if defined(USE_SDL2)
	int get_joyid_from_instanceID(SDL_JoystickID id);
# endif
 public:
	JoyThreadClass(std::shared_ptr<EmuThreadClassBase>  p, std::shared_ptr<USING_FLAGS> pflags, config_t *cfg, QObject *parent = 0);
	~JoyThreadClass();
	void run() { doWork("");}
	void SetEmu(std::shared_ptr<EmuThreadClassBase>  p);
	bool search_joydb(QString str, QString& answer);
	bool search_joydb_by_guid(QString guid, QString& answer);
	QString make_guid(SDL_JoystickGUID guid);
	QString joystick_guid(int num);
	QString joystick_name(int num);
	QString default_joyassign();
	
	void debug_log(int level, int domain_num, const char *fmt, ...);
	void debug_log(int level, int domain_num, QString msg);
	
public slots:
	void doWork(const QString &);
	void doExit(void);
	void do_map_joy_num(int num, int assign);
	void do_set_emulate_dpad(int num, bool val);
	bool replace_joydb(QString after);
	bool replace_joydb_by_guid(QString guid, QString after);
	void do_replace_default_assign(QString new_assign);
	
	void do_start(QThread::Priority prio);	
	void do_set_priority(QThread::Priority prio);	
 signals:
	int sig_finished(void);
	int sig_debug_log(int, int, QString);
	int sig_state_dpad(int, bool);
	// ToDo
	int sig_update_joystick_guid(int, QString);
	int sig_update_joystick_name(int, QString);
};


QT_END_NAMESPACE

#endif
